#include "Frender/Frender.hh"
#include <glad/glad.h>
#include <iostream>

// Shaders embedded in C Source
#include "Frender/GLTools.hh"
#include "Frender/Shaders/Stage1Vert.h"
#include "Frender/Shaders/Stage1Frag.h"

Frender::Renderer::Renderer()
{

}

void Frender::Renderer::render(float delta)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    bulkRender();
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
    materials.push_back(mat);

    return &materials[materials.size()-1];
}

Frender::Material* Frender::Renderer::createMaterial(GLTools::Shader shader)
{
    Material mat;

    // TODO: All the OpenGL UBO stuff

    mat.shader = shader;
    mat.type = Detail;
    materials.push_back(mat);

    return &materials[materials.size()-1];
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
    r.mat = {mat, mat->handle, mat->shader};

    render_objects.push_back(r);
    uint32_t* index = new uint32_t(render_objects.size()-1);
    render_objects[*index].pos = index;

    return {index, this};
}