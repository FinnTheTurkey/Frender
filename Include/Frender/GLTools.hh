#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <glm/glm.hpp>
#include <vector>
#include <string>

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

        bool created;
    
    private:
        uint32_t program;
    };
}

#endif