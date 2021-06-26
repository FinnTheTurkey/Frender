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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Orthographic?
    projection = glm::perspective(fov_rad, (float)width/height, near_distance, far_distance);

    bulkRender();
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

Frender::Material* Frender::Renderer::createMaterial()
{
    Material mat;

    // TODO: All the OpenGL UBO stuff

    if (!stage1_bulk_shader.created)
    {
        stage1_bulk_shader = GLTools::Shader(BulkStage1VertSrc, BulkStage1FragSrc);
    }

    mat.shader = stage1_bulk_shader;
    mat.type = Bulk;
    mat.uniforms = GLTools::UniformBuffer(stage1_bulk_shader, "Material", {{"color", GLTools::Vec3, glm::vec3(1, 0, 0)}});
    mat.textures = GLTools::TextureManager(stage1_bulk_shader);
    materials.push_back(mat);

    return &materials[materials.size()-1];
}

Frender::Material* Frender::Renderer::createMaterial(GLTools::Shader shader)
{
    Material mat;

    // TODO: All the OpenGL UBO stuff

    mat.shader = shader;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(shader, "Material", {{"color", GLTools::Vec3, glm::vec3(1, 0, 0)}});
    mat.textures = GLTools::TextureManager(shader);
    materials.push_back(mat);

    return &materials[materials.size()-1];
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

Frender::RenderObjectRef Frender::Renderer::createRenderObject(MeshRef mesh, Material* mat, glm::mat4 transform)
{
    // TODO: Insert in a sorted fasion
    RenderObject r;
    r.transform = transform;
    r.mesh = meshes[mesh];
    r.mat = {mat, &mat->textures, mat->uniforms.getRef(), mat->shader};

    render_objects.push_back(r);
    uint32_t* index = new uint32_t(render_objects.size()-1);
    render_objects[*index].pos = index;

    return {index, this};
}
