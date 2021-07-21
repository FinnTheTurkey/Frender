#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <array>

#include <algorithm>

#include <glad/glad.h>

#define GLERRORCHECK() switch (glGetError()) \
    { \
    case(GL_INVALID_ENUM): std::cerr << __FILE__ <<":" << __LINE__ << " GL_INVALID_ENUM\n";break; \
    case(GL_INVALID_VALUE): std::cerr << __FILE__ <<":" << __LINE__ << " GL_INVALID_VALUE\n";break; \
    case(GL_INVALID_OPERATION): std::cerr << __FILE__ <<":" << __LINE__ << " GL_INVALID_OPERATION\n";break; \
    }

namespace Frender::GLTools
{
    struct Vertex
    {
        Vertex(float a, float b, float c,
            float nx, float ny, float nz,
            float tx, float ty,
            float tax, float tay, float taz,
            float btax, float btay, float btaz
            )
        : position(a, b, c),
        normals(nx, ny, nz),
        tex_coords(tx, ty),
        tangent(tax, tay, taz),
        bitangent(btax, btay, btaz) {}

        glm::vec3 position;
        glm::vec3 normals;
        glm::vec2 tex_coords;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    class MeshBuffer
    {
    public:
        MeshBuffer() {num_indices = -1;};
        MeshBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~MeshBuffer();
        void destroy();

        void enable();

        size_t num_indices;
    private:
        uint32_t vao;
        uint32_t vbo;
        uint32_t ebo;
    };

    enum BufferType
    {
        Element, Array
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
        virtual void addDependantVao(VertexArray* vao) {};
        virtual void removeDependantVao(VertexArray* vao) {};

        virtual _BufferInfo _getBufferInfo() const {return {0, Array, {}, 0};};
    };

    template <typename T>
    class Buffer;

    class VertexArray
    {
    public:
        VertexArray() {};

        void addBuffer(IBuffer* buff)
        {
            buffers.push_back(buff);
            buff->addDependantVao(this);
        }

        void addIndices(const std::vector<uint32_t>& indices);

        void bind();

        void enable();

        void draw(int instances);

        void destroy();

    private:
        uint32_t vao;
        uint32_t ebo;
        size_t index_count = 0;
        std::vector<IBuffer*> buffers;
    };

    template <typename T>
    class Buffer : IBuffer
    {
    public:
        Buffer() {};
        Buffer(BufferType type, const std::vector<_VertexAttribSize>& sizes, const std::vector<T> initial_data)
        :type(type), sizes(sizes)
        {
            // Calculate total size
            total_size = 0;
            for (auto i : sizes)
            {
                total_size += i.size;
            }

            // Create buffer
            glGenBuffers(1, &handle);
            glBindBuffer(GL_ARRAY_BUFFER, handle);

            if (initial_data.size() > 0)
            {
                glBufferData(GL_ARRAY_BUFFER, total_size * initial_data.size(), &initial_data[0], type == Element ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            }
            else
            {
                glBufferData(GL_ARRAY_BUFFER, total_size, NULL, type == Element ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            }

            if (type == Array)
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
            data[index] = item;

            // Stage the change
            staged_changes.push_back(index);
        }

        void pushBack(const T& item)
        {
            data.push_back(item);

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

                glBindBuffer(GL_ARRAY_BUFFER, handle);
                glBufferData(GL_ARRAY_BUFFER, total_size * data.size(), &data[0], type == Element ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

                for (auto i : vaos)
                {
                    // Update the VAO to match the new buffer
                    i->bind();
                }

                reallocate = false;
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, handle);

                if (staged_changes.size() > data.size() / 2)
                {
                    // At this point it's probably faster to just re-uploat it all
                    glBufferSubData(GL_ARRAY_BUFFER, 0, total_size * data.size(), &data[0]);
                }
                else
                {
                    for (auto i : staged_changes)
                    {
                        // Upload changes to GPU
                        glBufferSubData(GL_ARRAY_BUFFER, total_size * i, total_size, &data[i]);
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

    private:
        uint32_t handle;
        BufferType type;
        std::vector<_VertexAttribSize> sizes;
        std::vector<T> data;
        size_t total_size;

        std::vector<uint32_t> staged_changes;
        bool reallocate;

        std::vector<VertexArray*> vaos;
    };

    template <typename T>
    void Buffer<T>::addDependantVao(VertexArray* vao)
    {
        vaos.push_back(vao);
    }

    template <typename T>
    void Buffer<T>::removeDependantVao(VertexArray* vao)
    {
        // Please don't delete a file
        vaos.erase(std::remove(vaos.begin(), vaos.end(), vao), vaos.end());
    }

    class Shader
    {
    public:
        Shader() {created = false;};
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

        bool created;
        uint32_t program;
    
    private:
        uint32_t mvp_location;
        uint32_t m_location;
    };

    enum UniformTypes
    {
        Int, Float, Vec2, Vec3, Vec4, Mat4
    };

    typedef std::variant<std::monostate, int32_t, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4> UniformType;

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
        void enable();
        void destroy();

        uint32_t handle;
    };

    class UniformBuffer
    {
    public:
        UniformBuffer() {};
        UniformBuffer(Shader shader, std::string ub_name, std::vector<UniformRow> values_info);

        void set(const std::string& name, UniformType value);

        void enable();
        void destroy();

        UniformRef getRef() {return UniformRef {handle};};

    private:
        void setBufferValue(const _UniformRow& row);

        uint32_t handle;
        std::vector<_UniformRow> data;
    };

    /**
    Wrapper class for an OpenGL Texture. All textures must be 8 bit RGBA
    */
    class Texture
    {
    public:
        Texture() {};
        Texture(uint32_t handle): handle(handle) {}
        Texture(int width, int height, const unsigned char* data);

        void destroy();

        uint32_t handle;
    private:
        
    };

    struct _TexStore
    {
        bool exists;
        std::string name;
        Texture tex;
        uint32_t location;
    };

    /**
    Class that manages textures, I guess
    */
    class TextureManager
    {
    public:
        TextureManager(): size(0) {};
        TextureManager(Shader shader) : shader(shader), size(0) {}

        void set(const std::string& name, Texture tex);
        void set(int index, Texture tex);

        void enable();
    
    private:
        Shader shader;
        int size;

        std::array<_TexStore, 8> data;
    };

    enum TextureTypes
    {
        RGB8, RGBA8, RGBA16
    };

    /**
    Framebuffer class
    */
    class Framebuffer
    {
    public:
        Framebuffer() {};
        Framebuffer(int width, int height, const std::vector<TextureTypes>& textures);

        std::vector<Texture> getTexture();

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
}

#endif