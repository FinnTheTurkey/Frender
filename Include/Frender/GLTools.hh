#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <variant>
#include <array>

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