#include "Frender/GLTools.hh"
#include <glad/glad.h>

Frender::GLTools::MeshBuffer::MeshBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    // Create buffer
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    // TODO: Handle dynamically changing data

    // Insert vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Insert indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    // Tell OpenGL what the data we just gave it means
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // We have to store the # of indices so we know
    // how many points to draw
    num_indices = indices.size();
}

Frender::GLTools::MeshBuffer::~MeshBuffer()
{
    if (num_indices != -1)
    {
        // Delete our OpenGL objects
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
}

void Frender::GLTools::MeshBuffer::enable()
{
    glBindVertexArray(vao);
}