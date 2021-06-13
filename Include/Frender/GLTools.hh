#ifndef FRENDER_GLTOOLS_HH
#define FRENDER_GLTOOLS_HH

#include <glm/glm.hpp>
#include <vector>

namespace Frender::GLTools
{
    struct Vertex
    {
        // TODO: Normal, UV, tangent/bitangent
        glm::vec3 position;
    };

    class MeshBuffer
    {
    public:
        MeshBuffer() {num_indices = -1;};
        MeshBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~MeshBuffer();

        void enable();
    
    private:
        uint32_t vao;
        uint32_t vbo;
        uint32_t ebo;

        size_t num_indices;
    };
}

#endif