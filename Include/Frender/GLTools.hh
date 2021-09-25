#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <array>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <algorithm>

#include <glad/glad.h>

#define GLERRORCHECK()                                                                                                 \
    switch (glGetError())                                                                                              \
    {                                                                                                                  \
    case (GL_INVALID_ENUM):                                                                                            \
        std::cerr << __FILE__ << ":" << __LINE__ << " GL_INVALID_ENUM\n";                                              \
        break;                                                                                                         \
    case (GL_INVALID_VALUE):                                                                                           \
        std::cerr << __FILE__ << ":" << __LINE__ << " GL_INVALID_VALUE\n";                                             \
        break;                                                                                                         \
    case (GL_INVALID_OPERATION):                                                                                       \
        std::cerr << __FILE__ << ":" << __LINE__ << " GL_INVALID_OPERATION\n";                                         \
        break;                                                                                                         \
    }

#ifndef FRENDER_MAX_UNIFORM_ARRAY
#define FRENDER_MAX_UNIFORM_ARRAY 256
#endif

namespace Frender::GLTools
{
struct Vertex
{
    Vertex(float a, float b, float c, float nx, float ny, float nz, float tx, float ty, float tax = 0, float tay = 0,
           float taz = 0, float btax = 0, float btay = 0, float btaz = 0)
        : position(a, b, c), normals(nx, ny, nz), tex_coords(tx, ty), tangent(tax, tay, taz),
          bitangent(btax, btay, btaz)
    {
    }

    glm::vec3 position;
    glm::vec3 normals;
    glm::vec2 tex_coords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class MeshBuffer
{
  public:
    MeshBuffer()
    {
        num_indices = -1;
    };
    MeshBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~MeshBuffer();
    void destroy();

    void enable();
    void disable();

    size_t num_indices;

  private:
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
};

enum BufferType
{
    Static,
    Dynamic,
    Element
};

struct _VertexAttribSize
{
    size_t size;
    uint32_t count;
};

struct _BufferInfo
{
    uint32_t handle;
    BufferType type;
    std::vector<_VertexAttribSize> sizes;
    size_t total_size;
};

class VertexArray;

class IBuffer
{
  public:
    virtual void addDependantVao(VertexArray* vao){};
    virtual void removeDependantVao(VertexArray* vao){};

    virtual _BufferInfo _getBufferInfo() const
    {
        return {0, Dynamic, {}, 0};
    };
};

template <typename T> class Buffer;

class VertexArray
{
  public:
    VertexArray() : buffers({}), ebo(nullptr){};

    void addBuffer(IBuffer* buff)
    {
        buffers.push_back(buff);
        buff->addDependantVao(this);
    }

    void addIndices(Buffer<uint32_t>* buff, size_t size);

    void bind();

    void enable();

    void draw(int instances);

    void destroy();

  private:
    uint32_t vao;
    Buffer<uint32_t>* ebo;
    size_t index_count = 0;
    std::vector<IBuffer*> buffers;
    bool has_vao = false;
};

template <typename T> class Buffer : public IBuffer
{
  public:
    Buffer(){};
    Buffer(BufferType type, const std::vector<_VertexAttribSize>& sizes, const std::vector<T> initial_data)
        : type(type), sizes(sizes), staged_changes({})
    {
        // Calculate total size
        total_size = 0;
        for (auto i : sizes)
        {
            total_size += i.size;
        }

        // Create buffer
        glGenBuffers(1, &handle);
        glBindBuffer(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, handle);

        if (initial_data.size() > 0)
        {
            glBufferData(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, total_size * initial_data.size(),
                         &initial_data[0], type == Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }
        else
        {
            glBufferData(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, total_size, NULL,
                         type == Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        }

        if (type == Dynamic)
        {
            data = initial_data;
        }
    }

    void destroy()
    {
        glDeleteBuffers(1, &handle);
    }

    T get(int index) const
    {
        return data[index];
    }

    void set(int index, T item)
    {
        if (index > size() - 1)
        {
            std::cerr << "Error: Cannot set item bigger then size. Use pushBack instead. \n";
        }
        data[index] = item;

        // Stage the change
        staged_changes.push_back(index);
    }

    void pushBack(const T& item)
    {
        data.push_back(item);

        reallocate = true;
    }

    void pop(int index)
    {
        data.erase(data.begin() + index);

        reallocate = true;
    }

    /**
    Apply's all changes to the buffer and sends them to the gpu
    */
    void apply()
    {

        if (reallocate)
        {
            // We have to completely re-allocate the VRAM
            glDeleteBuffers(1, &handle);
            glGenBuffers(1, &handle);

            glBindBuffer(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, handle);
            glBufferData(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, total_size * data.size(),
                         &data[0], type == Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

            for (auto i : vaos)
            {
                // Update the VAO to match the new buffer
                GLERRORCHECK();
                i->bind();
                GLERRORCHECK();
            }

            reallocate = false;
        }
        else
        {
            glBindBuffer(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, handle);

            if (staged_changes.size() > data.size() / 2)
            {
                // At this point it's probably faster to just re-uploat it all
                glBufferSubData(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, 0,
                                total_size * data.size(), &data[0]);
            }
            else
            {
                for (auto i : staged_changes)
                {
                    // Upload changes to GPU
                    glBufferSubData(type == Element ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, total_size * i,
                                    total_size, &data[i]);
                }
            }
        }

        staged_changes = {};
    }

    size_t size() const
    {
        return data.size();
    }

    // TODO: Deletion

    _BufferInfo _getBufferInfo() const override
    {
        return {handle, type, sizes, total_size};
    }

    void addDependantVao(VertexArray* vao) override;
    void removeDependantVao(VertexArray* vao) override;

    std::vector<T> data;

  private:
    uint32_t handle;
    BufferType type;
    std::vector<_VertexAttribSize> sizes;
    size_t total_size;

    std::vector<uint32_t> staged_changes;
    bool reallocate;

    std::vector<VertexArray*> vaos;
};

template <typename T> void Buffer<T>::addDependantVao(VertexArray* vao)
{
    vaos.push_back(vao);
}

template <typename T> void Buffer<T>::removeDependantVao(VertexArray* vao)
{
    // Please don't delete a file
    vaos.erase(std::remove(vaos.begin(), vaos.end(), vao), vaos.end());
}

class Shader
{
  public:
    Shader()
    {
        created = false;
    };
    Shader(const std::string& vert, const std::string& frag);
    ~Shader();
    void destroy();

    void enable();

    void setUniforms(glm::mat4 mvp, glm::mat4 m);

    // For custom uniforms
    uint32_t getUniformLocation(const std::string& name);

    void setUniform(uint32_t loc, int value);
    void setUniform(uint32_t loc, float value);
    void setUniform(uint32_t loc, glm::vec3 value);
    void setUniform(uint32_t loc, glm::mat4 value);

    bool created;
    uint32_t program;

  private:
    uint32_t mvp_location;
    uint32_t m_location;
};

enum UniformTypes
{
    Int,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat4,
    Vec4Array
};

typedef std::variant<std::monostate, int32_t, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4,
                     std::array<glm::vec4, FRENDER_MAX_UNIFORM_ARRAY>>
    UniformType;

struct UniformRow
{
    std::string name;
    UniformTypes type;
    UniformType value;
};

struct _UniformRow
{
    std::string name;
    UniformTypes type;
    UniformType value;
    int offset;
};

class UniformRef
{
  public:
    void enable(int loc);
    void destroy();

    uint32_t handle;
};

class UniformBuffer
{
  public:
    UniformBuffer(){};
    UniformBuffer(Shader shader, std::string ub_name, std::vector<UniformRow> values_info, int loc = 0);

    void set(const std::string& name, UniformType value);
    void setArray(const std::string& name, int index, glm::vec4 value);

    template <typename T> T get(const std::string& name)
    {
        for (auto i : data)
        {
            if (i.name == name)
            {
                return std::get<T>(i.value);
            }
        }

        return T();
    }

    void enable(int loc);
    void destroy();

    UniformRef getRef()
    {
        return UniformRef{handle};
    };

  private:
    void setBufferValue(const _UniformRow& row);

    uint32_t handle;
    std::vector<_UniformRow> data;
};

enum TextureVarieties
{
    Texture2D = 0,
    CUBEMAP_PX = 1,
    CUBEMAP_NX = 2,
    CUBEMAP_PY = 3,
    CUBEMAP_NY = 4,
    CUBEMAP_PZ = 5,
    CUBEMAP_NZ = 6,
};

/**
Wrapper class for an OpenGL Texture. All textures must be 8 bit RGBA
*/
class Texture
{
  public:
    Texture(){};
    Texture(uint32_t handle) : handle(handle)
    {
    }
    Texture(int width, int height, const unsigned char* data, bool mipmap = false);
    Texture(int width, int height, const float* data, bool mipmap = false);

    void destroy();

    uint32_t handle;

  private:
};

Texture equirectangularToCubemap(Shader shader, Texture equi, bool mipmap = false, int width = 512, int height = 512);

Texture createCubemap(int width, int height, bool mipmap = false);

struct _TexStore
{
    bool exists;
    std::string name;
    Texture tex;
    uint32_t location;
    TextureVarieties vari;
};

/**
Class that manages textures, I guess
*/
class TextureManager
{
  public:
    TextureManager() : size(0){};
    TextureManager(Shader shader) : shader(shader), size(0)
    {
    }

    void set(const std::string& name, Texture tex);
    void set(const std::string& name, Texture tex, TextureVarieties vari);
    void set(int index, Texture tex);

    void enable();

  private:
    Shader shader;
    int size;

    std::array<_TexStore, 8> data;
};

enum TextureTypes
{
    RGB8,
    RGBA8,
    RGBA16
};

/**
Framebuffer class
*/
class Framebuffer
{
  public:
    Framebuffer(){};
    Framebuffer(int width, int height, const std::vector<std::pair<TextureVarieties, TextureTypes>>& textures);

    std::vector<Texture> getTexture();
    Texture changeTexture(int index, TextureVarieties vari, TextureTypes type);
    void setTexture(int index, TextureVarieties vari, TextureTypes type, Texture tx, int mip = 0);

    void enable();
    void disable();

    void destroy();

    uint32_t depth_stencil;
    uint32_t framebuffer;

  private:
    std::vector<uint32_t> colors;
    std::vector<uint32_t> gl_enums;

    // Settings
    int width;
    int height;
};

/**
Transfers the depth buffer from fba to fbb.
Note: Framebuffers B will be bound at the end of the function
*/
void transferDepthBuffer(Framebuffer* fba, Framebuffer* fbb, int width, int height);

long long getVramUsage();
} // namespace Frender::GLTools

#endif