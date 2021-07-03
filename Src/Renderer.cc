#include "Frender/Frender.hh"
#include <glad/glad.h>
#include <iostream>

// Shaders embedded in C Source
#include "Frender/GLTools.hh"
#include "Frender/Shaders/Stage1Vert.h"
#include "Frender/Shaders/Stage1Frag.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

Frender::Renderer::Renderer(int width, int height)
{
    setRenderResolution(width, height);
}

void Frender::Renderer::render(float delta)
{
    // Count framerate
    // frame_count ++;
    // elapsed_time += delta;
    // elapsed_frames ++;

    // if (elapsed_time > 0.25)
    // {
    //     // Calculate FPS
    //     frame_time = elapsed_time/elapsed_frames;
    //     frame_rate = 1.0 / frame_time;
    //     elapsed_time = 0;
    //     elapsed_frames = 0;
    // }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // TODO: Orthographic?
    projection = glm::perspective(fov_rad, (float)width/height, near_distance, far_distance);

    bulkRender();

    std::cout << Frender::GLTools::getVramUsage() << std::endl;
}

void Frender::Renderer::setCamera(const glm::mat4 &matrix)
{
    camera = matrix;
    inv_camera = glm::inverse(matrix);
}

void Frender::Renderer::setRenderResolution(int new_width, int new_height)
{
    // TODO: Create new framebuffers
    width = new_width;
    height = new_height;
}

uint32_t Frender::Renderer::createMaterial()
{
    Material mat;

    // TODO: All the OpenGL UBO stuff

    if (!stage1_bulk_shader.created)
    {
        stage1_bulk_shader = GLTools::Shader(BulkStage1VertSrc, BulkStage1FragSrc);
    }

    mat.shader = stage1_bulk_shader;
    mat.type = Bulk;
    mat.uniforms = GLTools::UniformBuffer(stage1_bulk_shader, "Material", {
        {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
        {"has_texture", GLTools::Int, 0}
    });
    mat.textures = GLTools::TextureManager(stage1_bulk_shader);
    materials.push_back(mat);

    return materials.size()-1;
}

uint32_t Frender::Renderer::createMaterial(GLTools::Shader shader)
{
    Material mat;

    // TODO: All the OpenGL UBO stuff

    mat.shader = shader;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(shader, "Material", {
        {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
        {"has_texture", GLTools::Int, 0}
    });
    mat.textures = GLTools::TextureManager(shader);
    materials.push_back(mat);

    return materials.size()-1;
}

Frender::Material* Frender::Renderer::getMaterial(uint32_t material)
{
    return &materials[material];
}

Frender::Texture Frender::Renderer::createTexture(int width, int height, const unsigned char *data)
{
    Texture tex(width, height, data);

    // Store it so we can safely remove it
    textures.push_back(tex);

    return tex;
}

Frender::MeshRef Frender::Renderer::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    meshes.push_back({vertices, indices});
    return meshes.size()-1;
}

Frender::RenderObjectRef Frender::Renderer::createRenderObject(MeshRef mesh, uint32_t mat, glm::mat4 transform)
{
    // TODO: Insert in a sorted fasion

    auto m = getMaterial(mat);

    RenderObject r;
    r.transform = transform;
    r.mesh = meshes[mesh];
    r.mat = {mat, m->uniforms.getRef(), m->shader};

    render_objects.push_back(r);
    uint32_t* index = new uint32_t(render_objects.size()-1);

    render_objects[(*index)].pos = index;

    return {index, this};
}

glm::mat4 Frender::RenderObjectRef::getTransform()
{
    return renderer->_getRenderObject(index)->transform;
}

void Frender::RenderObjectRef::setTransform(glm::mat4 t)
{
    renderer->_getRenderObject(index)->transform = t;
}