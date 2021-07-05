#include "Frender/GLTools.hh"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <variant>

// ====================================================================
// Mesh Buffer
// ====================================================================

static long long vram_usage = 0;

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

    vram_usage += vertices.size() * sizeof(Vertex);

    // Insert indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    vram_usage += indices.size() * sizeof(Vertex);

    // Tell OpenGL what the data we just gave it means
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

        // TODO: Reduce vram_usage
    }
}

void Frender::GLTools::MeshBuffer::enable()
{
    glBindVertexArray(vao);
}


// ====================================================================
// Shader
// ====================================================================

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

// ====================================================================
// Uniform Buffer
// ====================================================================

Frender::GLTools::UniformBuffer::UniformBuffer(Shader shader, std::string ub_name, std::vector<UniformRow> values_info)
{
    uint32_t index = glGetUniformBlockIndex(shader.program, ub_name.c_str());

    // Tell OpenGL that the UBO at binding point 0 is ub_name
    glUniformBlockBinding(shader.program, index, 0);

    // Get size of ubo
    int block_size;
    glGetActiveUniformBlockiv(shader.program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size);

    // Create buffer
    glGenBuffers(1, &handle);
    glBindBuffer(GL_UNIFORM_BUFFER, handle);

    // Convert names to c strings for OpenGL
    std::vector<const char*> names_for_gl;
    for (auto i : values_info)
    {
        // This is utterly horrible, yet nessesairy
        char* x = new char[i.name.length()];
        auto length = i.name.copy(x, i.name.length(), 0);
        x[length] = '\0';

        names_for_gl.push_back(x);
    }

    // This is getting the "id" of each of the uniforms in the ubo
    uint32_t uniform_indices[names_for_gl.size()];
    glGetUniformIndices(shader.program, names_for_gl.size(), &names_for_gl[0], uniform_indices);

    // Get actual offsets in memory
    int32_t offsets[names_for_gl.size()];
    glGetActiveUniformsiv(shader.program, names_for_gl.size(), uniform_indices, GL_UNIFORM_OFFSET, offsets);

    // Check to insure the uniforms types are correct on both sides
    int32_t types_from_gl[names_for_gl.size()];
    glGetActiveUniformsiv(shader.program, names_for_gl.size(), uniform_indices, GL_UNIFORM_TYPE, types_from_gl);

    for (int t = 0; t < values_info.size(); t++)
    {
        if ((values_info[t].type == Float && types_from_gl[t] != GL_FLOAT) ||
            (values_info[t].type == Int && types_from_gl[t] != GL_INT) ||
            (values_info[t].type == Vec2 && types_from_gl[t] != GL_FLOAT_VEC2) ||
            (values_info[t].type == Vec3 && types_from_gl[t] != GL_FLOAT_VEC3) ||
            (values_info[t].type == Vec4 && types_from_gl[t] != GL_FLOAT_VEC4) ||
            (values_info[t].type == Mat4 && types_from_gl[t] != GL_FLOAT_MAT4))
        {
            // Type mismatch!
            std::cerr << "Type mismatch in uniform buffer: Make sure the names and types in the shader and the material match up exactly. \n";
        }
    }

    // Size up the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_DYNAMIC_DRAW);

    vram_usage += block_size;

    // Bind this buffer to the binding point we specified at the top
    // Which also binds it to the ubo specified by the shader
    // This line must be called everytime we switch UBOs
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, handle);

    // Put all the data into the buffer
    for (int i = 0; i < values_info.size(); i++)
    {
        data.push_back({values_info[i].name, values_info[i].type, values_info[i].value, offsets[i]});

        setBufferValue(data[i]);
    }

    // Free the memory that I shouldn't've had to allocate in the first place
    for (auto i : names_for_gl)
    {
        delete[] i;
    }
}

void Frender::GLTools::UniformBuffer::setBufferValue(const _UniformRow& row)
{
    switch (row.type)
    {
        case (Int):
        {
            const int32_t* vi = &std::get<int32_t>(row.value);
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(int32_t), vi);
            break;
        }
        case (Float):
        {
            const float* vf = &std::get<float>(row.value);
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float), vf);
            break;
        }
        case (Vec2):
        {
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float) * 2, glm::value_ptr(std::get<glm::vec2>(row.value)));
            break;
        }
        case (Vec3):
        {
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float) * 3, glm::value_ptr(std::get<glm::vec3>(row.value)));
            break;
        }
        case (Vec4):
        {
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float) * 4, glm::value_ptr(std::get<glm::vec4>(row.value)));
            break;
        }
        case (Mat4):
        {
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float) * 16, glm::value_ptr(std::get<glm::mat4>(row.value)));
            break;
        }
    }
}

void Frender::GLTools::UniformBuffer::set(const std::string &name, UniformType value)
{
    // Horribly inefficient
    for (auto i : data)
    {
        if (i.name == name)
        {
            i.value = value;

            setBufferValue(i);
        }
    }
}

void Frender::GLTools::UniformBuffer::enable()
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, handle);
}

void Frender::GLTools::UniformRef::enable()
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, handle);
}

void Frender::GLTools::UniformBuffer::destroy()
{
    glDeleteBuffers(1, &handle);
    // TODO: decrement vram_usage variable
}

void Frender::GLTools::UniformRef::destroy()
{
    glDeleteBuffers(1, &handle);
    // TODO: decrement vram_usage variable
}

// ====================================================================
// Texture
// ====================================================================

Frender::GLTools::Texture::Texture(int width, int height, const unsigned char* data)
{
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    // Set options
    // TODO: Make these user editable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Add data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    vram_usage += width * height * 4;
}

void Frender::GLTools::Texture::destroy()
{
    glDeleteTextures(1, &handle);

    // TODO: decrement vram_usage variable
}

void Frender::GLTools::TextureManager::set(const std::string &name, Texture tex)
{
    uint32_t loc = glGetUniformLocation(shader.program, name.c_str());
    data[size] = {true, name, tex, loc};
    size ++;
}

void Frender::GLTools::TextureManager::enable()
{
    // TODO: Don't re-bind textures
    auto pos = GL_TEXTURE0;
    for (int i = 0; i < size; i++)
    {
        glActiveTexture(pos);
        glBindTexture(GL_TEXTURE_2D, data[i].tex.handle);
        glUniform1i(data[i].location, i);
        pos ++;
    }
}

long long Frender::GLTools::getVramUsage()
{
    return vram_usage;
}