#include "Frender/GLTools.hh"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include <cstring>
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // tex coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Tangents
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Bitangents
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

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

void Frender::GLTools::MeshBuffer::disable()
{
    glBindVertexArray(0);
}

// ====================================================================
// Vertex Array
// ====================================================================
void Frender::GLTools::VertexArray::bind()
{
    GLERRORCHECK();
    if (!has_vao)
    {
        glGenVertexArrays(1, &vao);
        GLERRORCHECK();
        has_vao = true;
    }

    glBindVertexArray(vao);
    GLERRORCHECK();

    uint32_t grand_total_size = 0;
    for (auto i : buffers)
    {
        grand_total_size += i->_getBufferInfo().total_size;
    }

    uint32_t buffer_count = 0;
    for (auto i : buffers)
    {
        glBindBuffer(GL_ARRAY_BUFFER, i->_getBufferInfo().handle);
        uint32_t cumulative_size = 0;

        for (auto s : i->_getBufferInfo().sizes)
        {
            glVertexAttribPointer(buffer_count, s.count, GL_FLOAT, GL_FALSE, i->_getBufferInfo().total_size, (void*)(1l + (cumulative_size - 1)));
            glEnableVertexAttribArray(buffer_count);
            GLERRORCHECK();

            if (i->_getBufferInfo().type == Dynamic)
            {
                glVertexAttribDivisor(buffer_count, 1);
                GLERRORCHECK();
            }

            buffer_count ++;
            cumulative_size += s.size;
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Bind EBO
    GLERRORCHECK();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->_getBufferInfo().handle);
    GLERRORCHECK();
}

void Frender::GLTools::VertexArray::addIndices(Buffer<uint32_t>* buff, size_t size)
{
    ebo = buff;

    // Bind index buffer
    // It will be bound with the bind function
    // GLERRORCHECK();
    // glBindVertexArray(vao);
    // GLERRORCHECK();
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->_getBufferInfo().handle);
    // GLERRORCHECK();

    index_count = size;
}

void Frender::GLTools::VertexArray::draw(int instances)
{
    // enable();
    glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, instances);
}

void Frender::GLTools::VertexArray::enable()
{
    glBindVertexArray(vao);
}

void Frender::GLTools::VertexArray::destroy()
{
    for (auto i : buffers)
    {
        i->removeDependantVao(this);
    }

    glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &ebo);
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

uint32_t Frender::GLTools::Shader::getUniformLocation(const std::string &name)
{
    return glGetUniformLocation(program, name.c_str());
}

void Frender::GLTools::Shader::setUniform(uint32_t loc, int value)
{
    glUniform1i(loc, value);
}

void Frender::GLTools::Shader::setUniform(uint32_t loc, float value)
{
    glUniform1f(loc, value);
}

void Frender::GLTools::Shader::setUniform(uint32_t loc, glm::vec3 value)
{
    glUniform3f(loc, value.x, value.y, value.z);
}

void Frender::GLTools::Shader::setUniform(uint32_t loc, glm::mat4 mat)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

// ====================================================================
// Uniform Buffer
// ====================================================================

Frender::GLTools::UniformBuffer::UniformBuffer(Shader shader, std::string ub_name, std::vector<UniformRow> values_info, int loc)
{
    uint32_t index = glGetUniformBlockIndex(shader.program, ub_name.c_str());

    // Tell OpenGL that the UBO at binding point loc is ub_name
    glUniformBlockBinding(shader.program, index, loc);

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
        char* x = new char[i.name.length()+1];
        // auto length = i.name.copy(x, i.name.length(), 0);
        // x[length+1] = '\0';
        std::strcpy(x, i.name.c_str());

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
            (values_info[t].type == Vec4Array && types_from_gl[t] != GL_FLOAT_VEC4) ||
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
        case (Vec4Array):
        {
            glBufferSubData(GL_UNIFORM_BUFFER, row.offset, sizeof(float) * 4 * FRENDER_MAX_UNIFORM_ARRAY, &std::get<std::array<glm::vec4, FRENDER_MAX_UNIFORM_ARRAY>>(row.value)[0]);
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

            glBindBuffer(GL_UNIFORM_BUFFER, handle);
            setBufferValue(i);
        }
    }
}

void Frender::GLTools::UniformBuffer::setArray(const std::string &name, int index, glm::vec4 value)
{
    // Horribly inefficient
    int e = 0;
    for (auto i : data)
    {
        if (i.name == name)
        {
            auto v = std::get<std::array<glm::vec4, FRENDER_MAX_UNIFORM_ARRAY>>(i.value);
            v[index] = value;
            i.value = v;
            data[e] = i;

            glBindBuffer(GL_UNIFORM_BUFFER, handle);
            setBufferValue(i);
        }
        e++;
    }
}

void Frender::GLTools::UniformBuffer::enable(int loc)
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, loc, handle);
}

void Frender::GLTools::UniformRef::enable(int loc)
{
    glBindBuffer(GL_UNIFORM_BUFFER, handle);
    glBindBufferBase(GL_UNIFORM_BUFFER, loc, handle);
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

Frender::GLTools::Texture::Texture(int width, int height, const unsigned char* data, bool mipmap)
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
    
    if (mipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    vram_usage += width * height * 4;
}

Frender::GLTools::Texture::Texture(int width, int height, const float* data, bool mipmap)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);

    if (mipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    vram_usage += width * height * 8;
}

void Frender::GLTools::Texture::destroy()
{
    glDeleteTextures(1, &handle);

    // TODO: decrement vram_usage variable
}

Frender::GLTools::Texture Frender::GLTools::createCubemap(int width, int height, bool mipmap)
{
    uint32_t tx;

    glGenTextures(1, &tx);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tx);

    for (unsigned int i = 0; i < 6; ++i)
    {
        // note that we store each face with 16 bit floating point values
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 
                    width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (mipmap)
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    return tx;
}

Frender::GLTools::Texture Frender::GLTools::equirectangularToCubemap(Shader shader, Texture equi, bool mipmap, int og_width, int og_height)
{

    // Create cube mesh
    std::vector<GLTools::Vertex> vertices = {
            // back face
            {-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // bottom-left
            {1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // top-right
            {1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f}, // bottom-right
            {1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f}, // top-right
            {-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f}, // bottom-left
            {-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f}, // top-left
            // front face
            {-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f}, // bottom-left
             {1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f}, // bottom-right
             {1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f}, // top-right
            { 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f}, // top-right
            {-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f}, // top-left
            {-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f}, // bottom-left
            // left face
            {-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // top-right
            {-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // top-left
            {-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // bottom-left
            {-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // bottom-left
            {-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // bottom-right
            {-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // top-right
            // right face
            { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // top-left
             {1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // bottom-right
             {1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f}, // top-right         
             {1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f}, // bottom-right
            { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f}, // top-left
            { 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f}, // bottom-left     
            // bottom face
            {-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f}, // top-right
            { 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f}, // top-left
            { 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f}, // bottom-left
            { 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f}, // bottom-left
            {-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f}, // bottom-right
            {-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f}, // top-right
            // top face
            {-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f}, // top-left
             {1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f}, // bottom-right
            { 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f}, // top-right     
             {1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f}, // bottom-right
            {-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f}, // top-left
            {-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f}  // bottom-left        
    };

    std::vector<uint32_t> indices;
    for (int i = 0; i < vertices.size(); i++)
    {
        indices.push_back(i);
    }

    auto mb = MeshBuffer(vertices, indices);
    mb.enable();

    shader.enable();
    auto t = TextureManager(shader);
    t.set("equirectangularMap", equi);
    t.enable();

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = 
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    shader.setUniform(shader.getUniformLocation("projection"), captureProjection);

    auto cubemap = createCubemap(og_width, og_height, mipmap);
    unsigned int max_mips = mipmap ? 5 : 1;

    for (unsigned int mip = 0; mip < max_mips; mip ++)
    {
        int width = og_height * std::pow(0.5, mip);
        int height = og_width * std::pow(0.5, mip);

        auto fb = Framebuffer(width, height, {
            {Texture2D, RGBA16},
        });

        auto sacrafice = fb.getTexture()[0];
        GLERRORCHECK();
        fb.setTexture(0, CUBEMAP_PX, RGBA16, cubemap, mip);

        t.enable();
        if (mipmap)
        {
            // Set roughness uniform
            shader.setUniform(shader.getUniformLocation("roughness"), (float)mip / (float)(max_mips - 1));
        }

        glViewport(0, 0, width, height);
        for (int i = 0; i < 6; i++)
        {
            shader.setUniform(shader.getUniformLocation("view"), captureViews[i]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mb.enable();

            glDrawElements(GL_TRIANGLES, mb.num_indices, GL_UNSIGNED_INT, 0);

            if (i != 5)
            {
                fb.setTexture(0, (TextureVarieties)(CUBEMAP_PX + i + 1), RGBA16, cubemap, mip);
            }
        }

        // Clean up
        // Destroying fbo destroys it's textures too, so add sacraficial texture
        fb.setTexture(0, Texture2D, RGBA16, sacrafice);
        fb.destroy();

        std::cout << "Did thingy for mip " << mip << "\n";
    }
    mb.destroy();

    return cubemap;
}

void Frender::GLTools::TextureManager::set(const std::string &name, Texture tex)
{
    set(name, tex, Texture2D);
}

void Frender::GLTools::TextureManager::set(const std::string &name, Texture tex, TextureVarieties vari)
{
    // Make sure this texture doesn't already exist
    int p = -1;
    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].name == name)
        {
            p = i;
            break;
        }
    }

    if (p == -1)
    {
        GLERRORCHECK();
        uint32_t loc = glGetUniformLocation(shader.program, name.c_str());
        GLERRORCHECK();
        data[size] = {true, name, tex, loc, vari};
        size ++;
    }
    else
    {
        set(p, tex);
    }
}

void Frender::GLTools::TextureManager::set(int index, Texture tex)
{
    data[index].tex = tex;
}

void Frender::GLTools::TextureManager::enable()
{
    // TODO: Don't re-bind textures
    auto pos = GL_TEXTURE0;
    for (int i = 0; i < size; i++)
    {
        glActiveTexture(pos);
        GLERRORCHECK();
        glBindTexture(data[i].vari == Texture2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, data[i].tex.handle);
        if (glGetError() == GL_INVALID_OPERATION)
        {
            std::cout << "Failed to bind texture\n";
        }
        if (data[i].vari != Texture2D)
        {
            // std::cout << "tesy\n";
        }
        GLERRORCHECK();
        glUniform1i(data[i].location, i);
        GLERRORCHECK();
        pos ++;
    }
}

long long Frender::GLTools::getVramUsage()
{
    return vram_usage;
}

// ====================================================================
// Framebuffer
// ====================================================================
Frender::GLTools::Framebuffer::Framebuffer(int width, int height, const std::vector<std::pair<TextureVarieties, TextureTypes>>& textures)
: width(width), height(height)
{
    GLERRORCHECK();
    
    // Create framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLERRORCHECK();

    // Resize array
    colors.resize(textures.size());
    gl_enums.resize(textures.size());

    int c = 0;
    for (auto i : textures)
    {
        changeTexture(c, i.first, i.second);
        GLERRORCHECK();
        c++;
    }

    // Create Renderbuffer for depth and stencil buffers
    // We may want to change this later
    glGenRenderbuffers(1, &depth_stencil);
    GLERRORCHECK();

    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
    GLERRORCHECK();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    GLERRORCHECK();
    // glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach depth and stencil renderbuffer to fbo
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil);
    GLERRORCHECK();

    // Tell OpenGL how many buffers there are
    glDrawBuffers(gl_enums.size(), &gl_enums[0]);
    GLERRORCHECK();

    // Check to make sure it worked correctly
    auto n = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (n != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error creating framebuffer: Framebuffer is incomplete\n";
        switch (n)
        {
        case (GL_FRAMEBUFFER_UNDEFINED): std::cerr << "GL_FRAMEBUFFER_UNDEFINED\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"; break;
        case (GL_FRAMEBUFFER_UNSUPPORTED): std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n"; break;
        case (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS): std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n"; break;
        }
    }

    // Normally, I wouldn't bother, but this seems important
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERRORCHECK();
}

Frender::GLTools::Texture Frender::GLTools::Framebuffer::changeTexture(int c, Frender::GLTools::TextureVarieties vari, Frender::GLTools::TextureTypes typ)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Get type of texture
    uint32_t type;
    uint32_t val;
    uint32_t variety;
    uint32_t variety2;
    switch (typ)
    {
    case (RGB8): type = GL_RGB; val = GL_UNSIGNED_BYTE; break;
    case (RGBA8): type = GL_RGBA; val = GL_UNSIGNED_BYTE; break;
    case (RGBA16): type = GL_RGBA16F; val = GL_FLOAT; break;
    }

    switch (vari)
    {
    case (Texture2D): variety = GL_TEXTURE_2D; variety2 = variety; break;
    default:
        variety2 = GL_TEXTURE_CUBE_MAP_POSITIVE_X + vari - 1; break;
        variety = GL_TEXTURE_CUBE_MAP;
    }

    GLERRORCHECK();

    // Create texture for color buffer
    uint32_t tx;
    glGenTextures(1, &tx);
    glBindTexture(variety, tx);

    GLERRORCHECK();

    // Allocate space (but don't fill it)
    glTexImage2D(variety, 0, type, width, height, 0, GL_RGBA, val, NULL);

    GLERRORCHECK();

    // Set parameters
    glTexParameteri(variety, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(variety, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(variety, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(variety, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (vari != Texture2D)
    {
        glTexParameteri(variety, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    // Attach color texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + c, variety2, tx, 0);
    GLERRORCHECK();

    // Add it to the array of enums
    gl_enums[c] = GL_COLOR_ATTACHMENT0 + c;
    colors[c] = tx;

    return tx;
}

void Frender::GLTools::Framebuffer::setTexture(int index, TextureVarieties vari, TextureTypes typ, Texture tx, int mip)
{

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLERRORCHECK();

    // Get type of texture
    uint32_t type;
    uint32_t val;
    uint32_t variety;
    uint32_t variety2;
    switch (typ)
    {
    case (RGB8): type = GL_RGB; val = GL_UNSIGNED_BYTE; break;
    case (RGBA8): type = GL_RGBA; val = GL_UNSIGNED_BYTE; break;
    case (RGBA16): type = GL_RGBA16F; val = GL_FLOAT; break;
    }

    switch (vari)
    {
    case (Texture2D): variety = GL_TEXTURE_2D; variety2 = variety; break;
    default:
        variety = GL_TEXTURE_CUBE_MAP;
        variety2 = GL_TEXTURE_CUBE_MAP_POSITIVE_X + vari - 1; break;
    }

    glBindTexture(variety, tx.handle);
    GLERRORCHECK();

    // Attach color texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, variety2, tx.handle, mip);
    GLERRORCHECK();
}

std::vector<Frender::GLTools::Texture> Frender::GLTools::Framebuffer::getTexture()
{
    std::vector<Frender::GLTools::Texture> txes;
    for (auto i : colors)
    {
        txes.push_back(i);
    }
    return txes;
}

void Frender::GLTools::Framebuffer::enable()
{
    GLERRORCHECK();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLERRORCHECK();
}

void Frender::GLTools::Framebuffer::disable()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // uint32_t attachments[1] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, attachments);
}

void Frender::GLTools::Framebuffer::destroy()
{
    glDeleteFramebuffers(1, &framebuffer);
    for (auto i : colors)
    {
        glDeleteTextures(1, &i);
    }
    glDeleteRenderbuffers(1, &depth_stencil);
}

void Frender::GLTools::transferDepthBuffer(Framebuffer* fba, Framebuffer* fbb, int width, int height)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fba->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbb->framebuffer);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // TODO: Maybe stencil buffer as well?

    fbb->enable();
}