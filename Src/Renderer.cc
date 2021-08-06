#include "Frender/Frender.hh"
#include <glad/glad.h>
#include <iostream>

#include "Frender/GLTools.hh"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

// Shaders embedded in C Source
#include "Frender/Shaders/Stage1Vert.h"
#include "Frender/Shaders/Stage1Frag.h"
#include "Frender/Shaders/UnlitVert.h"
#include "Frender/Shaders/UnlitFrag.h"
#include "Frender/Shaders/Stage2Vert.h"
#include "Frender/Shaders/Stage2Frag.h"
#include "Frender/Shaders/Stage2FragD.h"
#include "Frender/Shaders/Stage3Vert.h"
#include "Frender/Shaders/Stage3Frag.h"
#include "Frender/Shaders/Stage3FxaaFrag.h"
#include "Frender/Shaders/BloomFrag.h"
#include "Frender/Shaders/Sphere.h"

Frender::Renderer::Renderer(int width, int height)
{
    // Create stage3 shaders
    stage3_shader = GLTools::Shader(Stage3VertSrc, Stage3FragSrc);
    bloom_exposure_loc = stage3_shader.getUniformLocation("bloom_exposure");

    bloom_shader = GLTools::Shader(Stage3VertSrc, BloomFragSrc);
    bloom_horizontal_loc = bloom_shader.getUniformLocation("horizontal");

    stage3fxaa_shader = GLTools::Shader(Stage3VertSrc, Stage3FxaaFragSrc);
    bloom_exposure_loc_fxaa = stage3_shader.getUniformLocation("bloom_exposure");

    GLERRORCHECK();

    // Create stage2 shaders
    stage2_light_shader = GLTools::Shader(Stage2VertSrc, Stage2FragSrc);
    light_uniforms.width = stage2_light_shader.getUniformLocation("width");
    light_uniforms.height = stage2_light_shader.getUniformLocation("height");
    light_uniforms.color = stage2_light_shader.getUniformLocation("light_color");
    light_uniforms.cam_pos = stage2_light_shader.getUniformLocation("cam_pos");
    light_uniforms.light_pos = stage2_light_shader.getUniformLocation("light_pos");
    light_uniforms.radius = stage2_light_shader.getUniformLocation("radius");

    stage2_dlight_shader = GLTools::Shader(Stage3VertSrc, Stage2FragDSrc);
    dlight_uniforms.width = stage2_dlight_shader.getUniformLocation("width");
    dlight_uniforms.height = stage2_dlight_shader.getUniformLocation("height");
    dlight_uniforms.color = stage2_dlight_shader.getUniformLocation("light_color");
    dlight_uniforms.cam_pos = stage2_dlight_shader.getUniformLocation("cam_pos");
    dlight_uniforms.light_pos = stage2_dlight_shader.getUniformLocation("light_direction");

    // Create forward shaders
    unlit = GLTools::Shader(UnlitVertSrc, UnlitFragSrc);

    setRenderResolution(width, height);
    GLERRORCHECK();

    // Create plane
    std::vector<Frender::Vertex> vertices = {
        {1.0f,  1.0f, 0.0f , 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, // top right
        {1.0f, -1.0f, 0.0f , 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, // bottom right
        {-1.0f, -1.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // bottom left
        {-1.0f,  1.0f, 0.0f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}  // top left 
    };
    std::vector<uint32_t> indices = {  // note that we start from 0!
        3, 1, 0,   // first triangle
        3, 2, 1    // second triangle
    };

    plane = GLTools::MeshBuffer(vertices, indices);

    // Create light sphere
    light_sphere = GLTools::MeshBuffer(sphere_vertices, sphere_indices);

    // New method for making light spheres
    // TODO: Do this in a better way
    // That doesn't leak memory
    GLTools::Buffer<Vertex>* vertex_buffer = new GLTools::Buffer<Vertex>(GLTools::Static, {
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec2), 2},
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec3), 3}
    }, sphere_vertices);

    point_light_buffer = GLTools::Buffer<PointLight>(GLTools::Dynamic, {
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec3), 3},
        {sizeof(float), 1},

        // Matrix has to be 4 values
        {sizeof(glm::vec4), 4},
        {sizeof(glm::vec4), 4},
        {sizeof(glm::vec4), 4},
        {sizeof(glm::vec4), 4}
    }, {});

    // point_light_buffer.pushBack({glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0});

    light_sphere_vao = GLTools::VertexArray();
    light_sphere_vao.addBuffer((GLTools::IBuffer*)vertex_buffer);
    light_sphere_vao.addBuffer((GLTools::IBuffer*)&point_light_buffer);

    // TODO: Don't leak memory
    auto sphere_index_buffer = new GLTools::Buffer<uint32_t>(GLTools::Element, {{sizeof(uint32_t), 1}}, sphere_indices);

    light_sphere_vao.addIndices(sphere_index_buffer, sphere_indices.size());
    GLERRORCHECK();
    light_sphere_vao.bind();
    GLERRORCHECK();
}

void Frender::Renderer::render(float delta)
{
    // Count framerate
    frame_count ++;
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
    // glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // TODO: Orthographic?
    projection = glm::perspective(fov_rad, (float)width/height, near_distance, far_distance);

    bulkRender();

    // std::cout << Frender::GLTools::getVramUsage() << std::endl;
    // std::cout << frame_count << "\n";
}

void Frender::Renderer::setCamera(const glm::mat4 &matrix)
{
    camera = matrix;
    inv_camera = glm::inverse(matrix);

    // Calculate frustum planes

}

void Frender::Renderer::setRenderResolution(int new_width, int new_height)
{
    width = new_width;
    height = new_height;

    // Create stage3 fbo and textures
    if (has_stage3)
    {
        stage3_fbo.destroy();

        if (bloom_res_scale != 0)
        {
            bloom_fbo1.destroy();
            bloom_fbo2.destroy();
        }
        GLERRORCHECK();
    }

    stage3_fbo = GLTools::Framebuffer(width, height, {
        GLTools::TextureTypes::RGBA16,
        GLTools::TextureTypes::RGBA16
    });
    GLERRORCHECK();
    stage3_tex = GLTools::TextureManager(stage3_shader);
    GLERRORCHECK();
    stage3_tex.set("frame", stage3_fbo.getTexture()[0]);
    stage3_tex.set("brightness", stage3_fbo.getTexture()[1]);
    GLERRORCHECK();
    has_stage3 = true;

    // Create bloom FBOs
    if (bloom_res_scale != 0)
    {
        bloom_fbo1 = GLTools::Framebuffer(bloom_res_scale * width, bloom_res_scale * height, {
            GLTools::TextureTypes::RGBA16
        });

        bloom_fbo2 = GLTools::Framebuffer(bloom_res_scale * width, bloom_res_scale * height, {
            GLTools::TextureTypes::RGBA16
        });

        bloom_tex1 = GLTools::TextureManager(bloom_shader);
        bloom_tex1.set("frame", bloom_fbo2.getTexture()[0]);

        bloom_tex2 = GLTools::TextureManager(bloom_shader);
        bloom_tex2.set("frame", bloom_fbo1.getTexture()[0]);

        stage3_tex.set("bloom_blur", bloom_fbo2.getTexture()[0]);
    }

    // Create stage2 fbo and textures
    if (has_stage2)
    {
        stage2_fbo.destroy();
        GLERRORCHECK();
    }
    
    stage2_fbo = GLTools::Framebuffer(width, height, {
        GLTools::TextureTypes::RGBA8,
        GLTools::TextureTypes::RGBA16,
        GLTools::TextureTypes::RGBA16
    });
    GLERRORCHECK();
    stage2_tex = GLTools::TextureManager(stage2_light_shader);
    GLERRORCHECK();
    stage2_tex.set("ColorRoughness", stage2_fbo.getTexture()[0]);
    stage2_tex.set("NormalMetal", stage2_fbo.getTexture()[1]);
    stage2_tex.set("position", stage2_fbo.getTexture()[2]);
    GLERRORCHECK();

    stage2_texd = GLTools::TextureManager(stage2_dlight_shader);
    GLERRORCHECK();
    stage2_texd.set("ColorRoughness", stage2_fbo.getTexture()[0]);
    stage2_texd.set("NormalMetal", stage2_fbo.getTexture()[1]);
    stage2_texd.set("position", stage2_fbo.getTexture()[2]);
    GLERRORCHECK();

    has_stage3 = true;
}

uint32_t Frender::Renderer::createMaterial()
{
    Material mat;

    if (!stage1_bulk_shader.created)
    {
        stage1_bulk_shader = GLTools::Shader(BulkStage1VertSrc, BulkStage1FragSrc);
    }

    mat.shader = stage1_bulk_shader;
    mat.type = Bulk;
    mat.uniforms = GLTools::UniformBuffer(stage1_bulk_shader, "Material", {
        {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
        {"roughness", GLTools::Float, 0.4f},
        {"metalness", GLTools::Float, 0.0f},
        {"has_diffuse_map", GLTools::Int, 0},
        {"has_normal_map", GLTools::Int, 0},
        {"has_roughness_map", GLTools::Int, 0},
        {"has_metal_map", GLTools::Int, 0}
    });
    mat.textures = GLTools::TextureManager(stage1_bulk_shader);
    materials.push_back(mat);

    return materials.size()-1;
}

uint32_t Frender::Renderer::createMaterial(GLTools::Shader shader)
{
    Material mat;

    mat.shader = shader;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(shader, "Material", {
        {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
        {"roughness", GLTools::Float, 0.4f},
        {"metalness", GLTools::Float, 0.0f},
        {"has_diffuse_map", GLTools::Int, 0},
        {"has_normal_map", GLTools::Int, 0},
        {"has_roughness_map", GLTools::Int, 0},
        {"has_metal_map", GLTools::Int, 0}
    });
    mat.textures = GLTools::TextureManager(shader);
    materials.push_back(mat);

    return materials.size()-1;
}

uint32_t Frender::Renderer::createUnlitMaterial(float emmisive)
{
    Material mat;

    mat.shader = unlit;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(unlit, "Material", {
        {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
        {"emmisive", GLTools::Float, emmisive},
        {"has_diffuse_map", GLTools::Int, 0},
    });
    mat.textures = GLTools::TextureManager(unlit);
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

Frender::MeshRef Frender::Renderer::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& nindices)
{
    // meshes.push_back({vertices, indices});
    GLTools::Buffer<Vertex>* vertex_buffer = new GLTools::Buffer<Vertex>(GLTools::Static, {
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec2), 2},
        {sizeof(glm::vec3), 3},
        {sizeof(glm::vec3), 3}
    }, vertices);

    auto index_buffer = new GLTools::Buffer<uint32_t>(GLTools::Element, {{sizeof(uint32_t), 1}}, nindices);

    meshes.push_back(vertex_buffer);
    indices.push_back({nindices.size(), index_buffer});

    // Generate bounding box
    auto min_pos = glm::vec3(vertices[0].position.x, vertices[0].position.y, vertices[0].position.z);
    auto max_pos = glm::vec3(vertices[0].position.x, vertices[0].position.y, vertices[0].position.z);

    for (int i = 0; i < vertices.size(); i++)
    {
        auto point = vertices[i].position;

        // Do each X, Y, and Z individually to make sure we cover everything
        if (point.x < min_pos.x)
        {
            min_pos.x = point.x;
        }
        if (point.y < min_pos.y)
        {
            min_pos.y = point.y;
        }
        if (point.z < min_pos.z)
        {
            min_pos.z = point.z;
        }

        if (point.x > max_pos.x)
        {
            max_pos.x = point.x;
        }
        if (point.y > max_pos.y)
        {
            max_pos.y = point.y;
        }
        if (point.z > max_pos.z)
        {
            max_pos.z = point.z;
        }
    }

    bounding_boxes.push_back({min_pos, max_pos, min_pos, max_pos});

    return meshes.size()-1;
}

Frender::RenderObjectRef Frender::Renderer::createRenderObject(MeshRef mesh, uint32_t mat, glm::mat4 transform)
{
    auto m = getMaterial(mat);

    if (m->shader.program != stage1_bulk_shader.program)
    {
        // Use unlit pipeline
        return createUnlitRenderObject(m->shader, mesh, mat, transform);
    }

    // Find the correct section, of create if it doesn't exist
    int mat_section_index = -1;
    int c = 0;
    for (auto i : scene_tree)
    {
        if (i.mat.mat_ref == mat)
        {
            mat_section_index = c;
        }
        c++;
    }

    if (mat_section_index == -1)
    {
        // Add a new mat section
        scene_tree.push_back(MatSection<MeshSection<ROInfo, ROInfoGPU>> {{mat, m->uniforms.getRef(), m->shader}, {}});
        mat_section_index = scene_tree.size() - 1;
    }

    MatSection<MeshSection<ROInfo, ROInfoGPU>>* ms = &scene_tree[mat_section_index];

    // Find Mesh Section
    int mesh_section_index = -1;
    c = 0;
    for (auto i : ms->meshes)
    {
        if (i.mesh == mesh)
        {
            mesh_section_index = c;
        }
        c++;
    }

    if (mesh_section_index == -1)
    {
        // Add a new mesh section
        // We have to create some OpenGL Objects for this one

        // Create buffer for renderobjects
        GLTools::Buffer<ROInfoGPU>* gpu_buffer = new GLTools::Buffer<ROInfoGPU>(GLTools::Dynamic, {
            // Matrix has to be 4 values
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},

            // Matrix has to be 4 values
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4}
        }, {});

        auto v = new GLTools::VertexArray();
        ms->meshes.push_back(MeshSection<ROInfo, ROInfoGPU> {mesh, v, {}, gpu_buffer});
        int index = ms->meshes.size() - 1;

        GLERRORCHECK();
        ms->meshes[index].vao->addBuffer(meshes[mesh]);
        GLERRORCHECK();
        ms->meshes[index].vao->addBuffer(gpu_buffer);
        GLERRORCHECK();
        ms->meshes[index].vao->addIndices(indices[mesh].second, indices[mesh].first);
        GLERRORCHECK();
        ms->meshes[index].vao->bind();
        GLERRORCHECK();
        
        mesh_section_index = ms->meshes.size() - 1;
    }

    MeshSection<ROInfo, ROInfoGPU>* mes = &ms->meshes[mesh_section_index];

    // Add actual data to CPU and GPU
    int index = mes->cpu_info.size();
    mes->cpu_info.push_back({index, transform});
    mes->gpu_buffer->pushBack({transform, transform});

    return {Lit, 0, mat_section_index, mesh_section_index, index, this};
}

Frender::RenderObjectRef Frender::Renderer::createUnlitRenderObject(GLTools::Shader shader, MeshRef mesh, uint32_t mat, glm::mat4 transform)
{
    // Find the correct shader section, and create if it doesn't exist
    int shader_section_index = -1;
    int c = 0;
    for (auto i : funlit_scene_tree)
    {
        if (i.shader.program == shader.program)
        {
            shader_section_index = c;
        }
        c++;
    }

    if (shader_section_index == -1)
    {
        funlit_scene_tree.push_back({shader, {}});
        shader_section_index = funlit_scene_tree.size() - 1;
    }

    auto m = getMaterial(mat);

    // Find the correct section, or create if it doesn't exist
    int mat_section_index = -1;
    c = 0;
    for (auto i : funlit_scene_tree[shader_section_index].mats)
    {
        if (i.mat.mat_ref == mat)
        {
            mat_section_index = c;
        }
        c++;
    }

    if (mat_section_index == -1)
    {
        // Add a new mat section
        funlit_scene_tree[shader_section_index].mats.push_back(MatSection<MeshSection<ROInfo, ROInfoGPU>> {{mat, m->uniforms.getRef(), m->shader}, {}});
        mat_section_index = funlit_scene_tree[shader_section_index].mats.size() - 1;
    }

    MatSection<MeshSection<ROInfo, ROInfoGPU>>* ms = &funlit_scene_tree[shader_section_index].mats[mat_section_index];

    // Find Mesh Section
    int mesh_section_index = -1;
    c = 0;
    for (auto i : ms->meshes)
    {
        if (i.mesh == mesh)
        {
            mesh_section_index = c;
        }
        c++;
    }

    if (mesh_section_index == -1)
    {
        // Add a new mesh section
        // We have to create some OpenGL Objects for this one

        // Create buffer for renderobjects
        GLTools::Buffer<ROInfoGPU>* gpu_buffer = new GLTools::Buffer<ROInfoGPU>(GLTools::Dynamic, {
            // Matrix has to be 4 values
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},

            // Matrix has to be 4 values
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4},
            {sizeof(glm::vec4), 4}
        }, {});

        auto v = new GLTools::VertexArray();
        ms->meshes.push_back(MeshSection<ROInfo, ROInfoGPU> {mesh, v, {}, gpu_buffer});
        int index = ms->meshes.size() - 1;

        GLERRORCHECK();
        ms->meshes[index].vao->addBuffer(meshes[mesh]);
        GLERRORCHECK();
        ms->meshes[index].vao->addBuffer(gpu_buffer);
        GLERRORCHECK();
        ms->meshes[index].vao->addIndices(indices[mesh].second, indices[mesh].first);
        GLERRORCHECK();
        ms->meshes[index].vao->bind();
        GLERRORCHECK();
        
        mesh_section_index = ms->meshes.size() - 1;
    }

    MeshSection<ROInfo, ROInfoGPU>* mes = &ms->meshes[mesh_section_index];

    // Add actual data to CPU and GPU
    int index = mes->cpu_info.size();
    mes->cpu_info.push_back({index, transform});
    mes->gpu_buffer->pushBack({transform, transform});

    return {Unlit, shader_section_index, mat_section_index, mesh_section_index, index, this};
}

Frender::RenderObjectRef Frender::Renderer::duplicateRenderObject(RenderObjectRef ro)
{
    if (ro.type == Lit)
    {
        return createRenderObject(scene_tree[ro.mat_section].meshes[ro.mesh_section].mesh, scene_tree[ro.mat_section].mat.mat_ref, scene_tree[ro.mat_section].meshes[ro.mesh_section].cpu_info[ro.index].model);
    }
    else
    {
        return createUnlitRenderObject(funlit_scene_tree[ro.shader_section].shader, funlit_scene_tree[ro.shader_section].mats[ro.mat_section].meshes[ro.mesh_section].mesh,
            funlit_scene_tree[ro.shader_section].mats[ro.mat_section].mat.mat_ref, funlit_scene_tree[ro.shader_section].mats[ro.mat_section].meshes[ro.mesh_section].cpu_info[ro.index].model);
    }
}

Frender::RenderObjectTraits Frender::Renderer::getRenderObjectTraits(RenderObjectRef ro)
{
    if (ro.type == Lit)
    {
        return {ro.type,
            stage1_bulk_shader, scene_tree[ro.mat_section].mat.mat_ref,
            scene_tree[ro.mat_section].meshes[ro.mesh_section].mesh,
            scene_tree[ro.mat_section].meshes[ro.mesh_section].cpu_info[ro.index].model};
    }
    else
    {
        return {ro.type,
            funlit_scene_tree[ro.shader_section].shader,
            funlit_scene_tree[ro.shader_section].mats[ro.mat_section].mat.mat_ref,
            funlit_scene_tree[ro.shader_section].mats[ro.mat_section].meshes[ro.mesh_section].mesh,
            funlit_scene_tree[ro.shader_section].mats[ro.mat_section].meshes[ro.mesh_section].cpu_info[ro.index].model};
    }
}

uint32_t Frender::Renderer::createPointLight(glm::vec3 position, glm::vec3 color, float radius)
{
    radius *= 1.5;
    auto m = glm::scale(glm::translate(glm::mat4(), position), glm::vec3(radius));

    point_lights.push_back({color, position, radius, m});

    // New method: Add to buffer
    point_light_buffer.pushBack({color, position, radius, m});

    return point_lights.size()-1;
}

uint32_t Frender::Renderer::createDirectionalLight(glm::vec3 color, glm::vec3 direction)
{
    directional_lights.push_back({color, direction});
    return point_lights.size()-1;
}

glm::mat4 Frender::RenderObjectRef::getTransform()
{
    return renderer->_getRenderObject(type, shader_section, mat_section, mesh_section, index)->model;
}

void Frender::RenderObjectRef::setTransform(glm::mat4 t)
{
    renderer->_getRenderObject(type, shader_section, mat_section, mesh_section, index)->model = t;
}

Frender::RenderObjectRef Frender::RenderObjectRef::duplicate()
{
    return renderer->duplicateRenderObject(*this);
}

// TODO: Free memory allocated by the buffers, and generally clean up better