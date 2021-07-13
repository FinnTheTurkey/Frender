#include "Frender/Frender.hh"
#include <glad/glad.h>
#include <iostream>

#include "Frender/GLTools.hh"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

// Shaders embedded in C Source
#include "Frender/Shaders/Stage1Vert.h"
#include "Frender/Shaders/Stage1Frag.h"
#include "Frender/Shaders/Stage2Vert.h"
#include "Frender/Shaders/Stage2Frag.h"
#include "Frender/Shaders/Stage2FragD.h"
#include "Frender/Shaders/Stage3Vert.h"
#include "Frender/Shaders/Stage3Frag.h"
#include "Frender/Shaders/BloomFrag.h"
#include "Frender/Shaders/Sphere.h"

Frender::Renderer::Renderer(int width, int height)
{
    // Create stage3 shaders
    stage3_shader = GLTools::Shader(Stage3VertSrc, Stage3FragSrc);
    bloom_exposure_loc = stage3_shader.getUniformLocation("bloom_exposure");

    bloom_shader = GLTools::Shader(Stage3VertSrc, BloomFragSrc);
    bloom_horizontal_loc = bloom_shader.getUniformLocation("horizontal");
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

uint32_t Frender::Renderer::createPointLight(glm::vec3 position, glm::vec3 color, float radius)
{
    point_lights.push_back({color, position, radius});
    return point_lights.size()-1;
}

uint32_t Frender::Renderer::createDirectionalLight(glm::vec3 color, glm::vec3 direction)
{
    directional_lights.push_back({color, direction});
    return point_lights.size()-1;
}

glm::mat4 Frender::RenderObjectRef::getTransform()
{
    return renderer->_getRenderObject(index)->transform;
}

void Frender::RenderObjectRef::setTransform(glm::mat4 t)
{
    renderer->_getRenderObject(index)->transform = t;
}