#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <variant>

namespace Frender::GLTools
{
    struct Vertex
    {
        Vertex(float a, float b, float c): position(a, b, c) {}
        // TODO: Normal, UV, tangent/bitangent
        glm::vec3 position;
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

        uint32_t handle;
    };

    class UniformBuffer
    {
    public:
        UniformBuffer() {};
        UniformBuffer(Shader shader, std::string ub_name, std::vector<UniformRow> values_info);

        void set(const std::string& name, UniformType value);

        void enable();

        UniformRef getRef() {return UniformRef {handle};};

    private:
        void setBufferValue(const _UniformRow& row);

        uint32_t handle;
        std::vector<_UniformRow> data;
    };

    
}

#endif