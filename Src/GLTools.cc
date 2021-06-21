#include "Frender/GLTools.hh"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Frender::GLTools::MeshBuffer::MeshBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    // Create buffer
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

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
    // if (num_indices != -1)
    // {
    //     // Delete our OpenGL objects
    //     glDeleteBuffers(1, &vbo);
    //     glDeleteBuffers(1, &ebo);
    //     glDeleteVertexArrays(1, &vao);
    // }
}

void Frender::GLTools::MeshBuffer::destroy()
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

Frender::GLTools::Shader::Shader(const std::string& vert, const std::string& frag)
{
    uint32_t fragment, vertex;

    vertex = glCreateShader(GL_VERTEX_SHADER);

    // Compile shader
    // For some reason this is nessesairy
    const char* vs = vert.c_str();
    glShaderSource(vertex, 1, &vs, NULL);
    glCompileShader(vertex);

    // Check for errors
    int v_success;
    char v_log[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &v_success);

    if (!v_success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, v_log);
        std::cerr << "Vertex shader compilation failed: " << v_log << "\n";
        return;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile shader
    // For some reason this is nessesairy
    const char* fs = frag.c_str();
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(fragment);

    // Check for errors
    int f_success;
    char f_log[512];
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &f_success);

    if (!f_success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, f_log);
        std::cerr << "Fragment shader compilation failed: " << f_log << "\n";
        return;
    }

    // Shader program
    program = glCreateProgram();

    // Link shaders
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    // Check for errors
    int p_success;
    char p_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &p_success);

    if (!p_success)
    {
        glGetProgramInfoLog(program, 512, NULL, p_log);
        std::cerr << "Shader linking failed: " << p_log << "\n";
        return;
    }

    // Remove useless shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Get uniform locations
    mvp_location = glGetUniformLocation(program, "mvp");
    m_location = glGetUniformLocation(program, "model");

    created = true;
}

Frender::GLTools::Shader::~Shader()
{
    // glDeleteProgram(program);
}

void Frender::GLTools::Shader::destroy()
{
    glDeleteProgram(program);
}


void Frender::GLTools::Shader::enable()
{
    if (!created)
    {
        std::cerr << "Attempting to enable inexistant program\n" ;
    }
    glUseProgram(program);
}

void Frender::GLTools::Shader::setUniforms(glm::mat4 mvp, glm::mat4 m)
{
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::value_ptr(m));
}