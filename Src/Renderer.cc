#include "Frender/Frender.hh"
#include <algorithm>
#include <array>
#include <glad/glad.h>
#include <iostream>

#include "Frender/GLTools.hh"
#include "LookupTable.hh"
#include "glm/detail/type_vec.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

// Shaders embedded in C Source
#include "Frender/Shaders/BloomFrag.h"
#include "Frender/Shaders/EquiToCubemapFrag.h"
#include "Frender/Shaders/EquiToCubemapFrag_convolute.h"
#include "Frender/Shaders/EquiToCubemapFrag_prefilter.h"
#include "Frender/Shaders/EquiToCubemapVert.h"
#include "Frender/Shaders/IntegrateBRDFFrag.h"
#include "Frender/Shaders/LitFrag.h"
#include "Frender/Shaders/LitVert.h"
#include "Frender/Shaders/SkyboxFrag.h"
#include "Frender/Shaders/SkyboxVert.h"
#include "Frender/Shaders/Sphere.h"
#include "Frender/Shaders/Stage1Frag.h"
#include "Frender/Shaders/Stage1Vert.h"
#include "Frender/Shaders/Stage2Frag.h"
#include "Frender/Shaders/Stage2FragA.h"
#include "Frender/Shaders/Stage2FragD.h"
#include "Frender/Shaders/Stage2Vert.h"
#include "Frender/Shaders/Stage3Frag.h"
#include "Frender/Shaders/Stage3FxaaFrag.h"
#include "Frender/Shaders/Stage3Vert.h"
#include "Frender/Shaders/UnlitFrag.h"
#include "Frender/Shaders/UnlitVert.h"

Frender::Renderer::Renderer(int width, int height) : light_index(0)
{
    // Create stage3 shaders
    stage3_shader = GLTools::Shader(Stage3VertSrc, Stage3FragSrc);
    bloom_exposure_loc = stage3_shader.getUniformLocation("bloom_exposure");

    bloom_shader = GLTools::Shader(Stage3VertSrc, BloomFragSrc);
    bloom_horizontal_loc = bloom_shader.getUniformLocation("horizontal");

    stage3fxaa_shader = GLTools::Shader(Stage3VertSrc, Stage3FxaaFragSrc);
    bloom_exposure_loc_fxaa = stage3_shader.getUniformLocation("bloom_exposure");

    equiToCubemap_shader = GLTools::Shader(EquiToCubemapVertSrc, EquiToCubemapFragSrc);
    equiToCubemap_convolution_shader = GLTools::Shader(EquiToCubemapVertSrc, EquiToCubemap_convoluteFragSrc);
    equiToCubemap_prefilter_shader = GLTools::Shader(EquiToCubemapVertSrc, EquiToCubemap_prefilterFragSrc);
    skybox_shader = GLTools::Shader(SkyboxVertSrc, SkyboxFragSrc);
    skybox_textures = GLTools::TextureManager(skybox_shader);
    skybox_vp_loc = skybox_shader.getUniformLocation("vp");

    int* dummy = new int(1);
    dummy_texture = GLTools::Texture(1, 1, (unsigned char*)dummy, false);

    GLERRORCHECK();
#ifndef FLUX_NO_DEFFERED
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

    stage2_alight_shader = GLTools::Shader(Stage3VertSrc, Stage2FragASrc);
    alight_uniforms.width = stage2_alight_shader.getUniformLocation("width");
    alight_uniforms.height = stage2_alight_shader.getUniformLocation("height");
    alight_uniforms.cam_pos = stage2_alight_shader.getUniformLocation("cam_pos");
#endif
    // Create forward shaders
    unlit = GLTools::Shader(UnlitVertSrc, UnlitFragSrc);

    // lit_shader = GLTools::Shader(LitVertSrc, LitFragSrc);
    // lit_uniforms.cam_pos = lit_shader.getUniformLocation("cam_pos");

    // Create light buffer
    // light_buffer = GLTools::UniformBuffer(
    // lit_shader, "Lights",
    // {
    // {"light_pos_dir_rad", GLTools::Vec4Array, std::array<glm::vec4, FRENDER_MAX_UNIFORM_ARRAY>()},
    // {"light_color_type", GLTools::Vec4Array, std::array<glm::vec4, FRENDER_MAX_UNIFORM_ARRAY>()},
    // },
    // 1);

    // Create plane
    std::vector<Frender::Vertex> vertices = {
        {1.0f, 1.0f, 0.0f, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},   // top right
        {1.0f, -1.0f, 0.0f, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},  // bottom right
        {-1.0f, -1.0f, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // bottom left
        {-1.0f, 1.0f, 0.0f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}   // top left
    };
    std::vector<uint32_t> indices = {
        // note that we start from 0!
        3, 1, 0, // first triangle
        3, 2, 1  // second triangle
    };

    plane = GLTools::MeshBuffer(vertices, indices);

    // Create cube mesh
    std::vector<GLTools::Vertex> c_vertices = {
        // back face
        {-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f}, // bottom-left
        {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},   // top-right
        {1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},  // bottom-right
        {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},   // top-right
        {-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f}, // bottom-left
        {-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f},  // top-left
        // front face
        {-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, // bottom-left
        {1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},  // bottom-right
        {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},   // top-right
        {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},   // top-right
        {-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},  // top-left
        {-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, // bottom-left
        // left face
        {-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},   // top-right
        {-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},  // top-left
        {-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f}, // bottom-left
        {-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f}, // bottom-left
        {-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // bottom-right
        {-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},   // top-right
        // right face
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},   // top-left
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}, // bottom-right
        {1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},  // top-right
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}, // bottom-right
        {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},   // top-left
        {1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // bottom-left
        // bottom face
        {-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f}, // top-right
        {1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f},  // top-left
        {1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},   // bottom-left
        {1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},   // bottom-left
        {-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f},  // bottom-right
        {-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f}, // top-right
        // top face
        {-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // top-left
        {1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},   // bottom-right
        {1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},  // top-right
        {1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},   // bottom-right
        {-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // top-left
        {-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}   // bottom-left
    };

    std::vector<uint32_t> c_indices;
    for (int i = 0; i < c_vertices.size(); i++)
    {
        c_indices.push_back(i);
    }

    cube = GLTools::MeshBuffer(c_vertices, c_indices);

#ifndef FLUX_NO_DEFFERED
    // Create light sphere
    light_sphere = GLTools::MeshBuffer(sphere_vertices, sphere_indices);

    // New method for making light spheres
    // TODO: Do this in a better way
    // That doesn't leak memory
    GLTools::Buffer<Vertex>* vertex_buffer = new GLTools::Buffer<Vertex>(GLTools::Static,
                                                                         {{sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec2), 2},
                                                                          {sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec3), 3}},
                                                                         sphere_vertices);

    point_light_buffer = GLTools::Buffer<PointLight>(GLTools::Dynamic,
                                                     {{sizeof(glm::vec3), 3},
                                                      {sizeof(glm::vec3), 3},
                                                      {sizeof(float), 1},

                                                      // Matrix has to be 4 values
                                                      {sizeof(glm::vec4), 4},
                                                      {sizeof(glm::vec4), 4},
                                                      {sizeof(glm::vec4), 4},
                                                      {sizeof(glm::vec4), 4}},
                                                     {});

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
#endif

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create precomputed BRDF
    auto ibrdf_shader = GLTools::Shader(Stage3VertSrc, IntegrateBRDFFragSrc);
    auto fb = GLTools::Framebuffer(512, 512, {{GLTools::Texture2D, GLTools::RGBA16}});

    glViewport(0, 0, 512, 512);

    auto sacrafice = fb.getTexture()[0];
    brdf_precomputed = Texture(512, 512, (float*)nullptr);
    fb.setTexture(0, GLTools::Texture2D, GLTools::RGBA16, brdf_precomputed);

    fb.enable();
    ibrdf_shader.enable();
    plane.enable();
    glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
    plane.disable();

    fb.setTexture(0, GLTools::Texture2D, GLTools::RGBA16, sacrafice);
    fb.disable();
    fb.destroy();
    ibrdf_shader.destroy();
    delete dummy;

    setRenderResolution(width, height);
    GLERRORCHECK();
}

void Frender::Renderer::render(float delta)
{
    // Count framerate
    frame_count++;
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

    glClearColor(0.0f, 0.8f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // TODO: Orthographic?
    projection = glm::perspective(fov_rad, (float)width / height, near_distance, far_distance);

    bulkRender();

    // std::cout << Frender::GLTools::getVramUsage() << std::endl;
    // std::cout << frame_count << "\n";
}

void Frender::Renderer::setCamera(const glm::mat4& matrix)
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

    stage3_fbo = GLTools::Framebuffer(
        width, height,
        {{GLTools::Texture2D, GLTools::TextureTypes::RGBA16}, {GLTools::Texture2D, GLTools::TextureTypes::RGBA16}});
    GLERRORCHECK();
    stage3_tex = GLTools::TextureManager(stage3_shader);
    GLERRORCHECK();
    stage3_tex.set("frame", stage3_fbo.getTexture()[0]);
    // stage3_tex.set("brightness", stage3_fbo.getTexture()[1]);
    GLERRORCHECK();
    has_stage3 = true;

    // Create bloom FBOs
    if (bloom_res_scale != 0)
    {
        bloom_fbo1 = GLTools::Framebuffer(bloom_res_scale * width, bloom_res_scale * height,
                                          {{GLTools::Texture2D, GLTools::TextureTypes::RGBA16}});

        bloom_fbo2 = GLTools::Framebuffer(bloom_res_scale * width, bloom_res_scale * height,
                                          {{GLTools::Texture2D, GLTools::TextureTypes::RGBA16}});

        bloom_tex1 = GLTools::TextureManager(bloom_shader);
        bloom_tex1.set("frame", bloom_fbo2.getTexture()[0]);

        bloom_tex2 = GLTools::TextureManager(bloom_shader);
        bloom_tex2.set("frame", bloom_fbo1.getTexture()[0]);

        stage3_tex.set("bloom_blur", bloom_fbo2.getTexture()[0]);
    }

#ifndef FLUX_NO_DEFFERED
    // Create stage2 fbo and textures
    if (has_stage2)
    {
        stage2_fbo.destroy();
        GLERRORCHECK();
    }

    stage2_fbo = GLTools::Framebuffer(width, height,
                                      {{GLTools::Texture2D, GLTools::TextureTypes::RGBA8},
                                       {GLTools::Texture2D, GLTools::TextureTypes::RGBA16},
                                       {GLTools::Texture2D, GLTools::TextureTypes::RGBA16}});
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

    alight_tx = GLTools::TextureManager(stage2_alight_shader);
    GLERRORCHECK();
    alight_tx.set("ColorRoughness", stage2_fbo.getTexture()[0]);
    alight_tx.set("NormalMetal", stage2_fbo.getTexture()[1]);
    alight_tx.set("position", stage2_fbo.getTexture()[2]);
    alight_tx.set("irradiance_map", irradiance_cubemap, GLTools::CUBEMAP_PX);
    alight_tx.set("prefilter_map", prefilter_cubemap, GLTools::CUBEMAP_PX);
    alight_tx.set("brdf", brdf_precomputed, GLTools::Texture2D);
    GLERRORCHECK();
#endif

    has_stage3 = true;
}

uint32_t Frender::Renderer::createMaterial()
{

#ifdef FRENDER_NO_DEFERED
    // return createLitMaterial();
#endif

    Material mat;

    if (!stage1_bulk_shader.created)
    {
        stage1_bulk_shader = GLTools::Shader(BulkStage1VertSrc, BulkStage1FragSrc);
    }
    mat.shader = stage1_bulk_shader;
    mat.type = Bulk;
    mat.uniforms = GLTools::UniformBuffer(stage1_bulk_shader, "Material",
                                          {{"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
                                           {"roughness", GLTools::Float, 0.4f},
                                           {"metalness", GLTools::Float, 0.0f},
                                           {"has_diffuse_map", GLTools::Int, 0},
                                           {"has_normal_map", GLTools::Int, 0},
                                           {"has_roughness_map", GLTools::Int, 0},
                                           {"has_metal_map", GLTools::Int, 0}});
    mat.textures = GLTools::TextureManager(stage1_bulk_shader);
    mat.textures.set("diffuse_map", dummy_texture);
    mat.textures.set("metal_map", dummy_texture);
    mat.textures.set("normal_map", dummy_texture);
    mat.textures.set("roughness_map", dummy_texture);
    materials.push_back(mat);

    return materials.size() - 1;
}

uint32_t Frender::Renderer::createMaterial(GLTools::Shader shader)
{
    Material mat;

    mat.shader = shader;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(shader, "Material",
                                          {{"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
                                           {"roughness", GLTools::Float, 0.4f},
                                           {"metalness", GLTools::Float, 0.0f},
                                           {"has_diffuse_map", GLTools::Int, 0},
                                           {"has_normal_map", GLTools::Int, 0},
                                           {"has_roughness_map", GLTools::Int, 0},
                                           {"has_metal_map", GLTools::Int, 0}});
    mat.textures = GLTools::TextureManager(shader);
    mat.textures.set("irradiance_map", irradiance_cubemap, GLTools::CUBEMAP_PX);
    mat.textures.set("prefilter_map", prefilter_cubemap, GLTools::CUBEMAP_PX);
    mat.textures.set("brdf", brdf_precomputed, GLTools::Texture2D);
    mat.textures.set("diffuse_map", dummy_texture);
    mat.textures.set("metal_map", dummy_texture);
    mat.textures.set("normal_map", dummy_texture);
    mat.textures.set("roughness_map", dummy_texture);
    materials.push_back(mat);

    return materials.size() - 1;
}

uint32_t Frender::Renderer::createUnlitMaterial(float emmisive)
{
    Material mat;

    mat.shader = unlit;
    mat.type = Detail;
    mat.uniforms = GLTools::UniformBuffer(unlit, "Material",
                                          {
                                              {"color", GLTools::Vec3, glm::vec3(1, 0, 0)},
                                              {"emmisive", GLTools::Float, emmisive},
                                              {"has_diffuse_map", GLTools::Int, 0},
                                          });
    mat.textures = GLTools::TextureManager(unlit);
    mat.textures.set("diffuse_map", dummy_texture);
    materials.push_back(mat);

    return materials.size() - 1;
}

Frender::Material* Frender::Renderer::getMaterial(uint32_t material)
{
    return &materials[material];
}

Frender::Texture Frender::Renderer::createTexture(int width, int height, const unsigned char* data)
{
    Texture tex(width, height, data, true);

    // Store it so we can safely remove it
    textures.push_back(tex);

    return tex;
}

Frender::MeshRef Frender::Renderer::createMesh(const std::vector<Vertex>& vertices,
                                               const std::vector<uint32_t>& nindices)
{
    // meshes.push_back({vertices, indices});
    GLTools::Buffer<Vertex>* vertex_buffer = new GLTools::Buffer<Vertex>(GLTools::Static,
                                                                         {{sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec2), 2},
                                                                          {sizeof(glm::vec3), 3},
                                                                          {sizeof(glm::vec3), 3}},
                                                                         vertices);

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

    return meshes.size() - 1;
}

Frender::RenderObjectRef Frender::Renderer::createRenderObject(MeshRef mesh, uint32_t mat, glm::mat4 transform)
{
    // return createLitRenderObject(mesh, mat, transform);
    if (getMaterial(mat)->shader.program != stage1_bulk_shader.program)
    {
        return createUnlitRenderObject(getMaterial(mat)->shader, mesh, mat, transform);
    }

    auto obj = _addRenderObject<ROInfo, ROInfoGPU>(scene_tree, {-1, 0, transform}, {transform, transform},
                                                   stage1_bulk_shader, mesh, mat, transform);
    obj.type = Lit;
    return obj;
}

Frender::RenderObjectRef Frender::Renderer::createUnlitRenderObject(GLTools::Shader shader, MeshRef mesh, uint32_t mat,
                                                                    glm::mat4 transform)
{

    auto obj = _addRenderObject<ROInfo, ROInfoGPU>(funlit_scene_tree, {-1, 0, transform}, {transform, transform},
                                                   shader, mesh, mat, transform);
    obj.type = Unlit;
    return obj;
}

template <typename ROCpu, typename ROGpu>
Frender::RenderObjectRef Frender::Renderer::_addRenderObject(
    std::vector<ShaderSection<MatSection<MeshSection<ROCpu, ROGpu>>>>& scene, ROCpu cpu, ROGpu gpu,
    GLTools::Shader shader, MeshRef mesh, uint32_t mat, glm::mat4 transform, int rows)
{
    // Find the correct shader section, and create if it doesn't exist
    int shader_section_index = -1;
    int c = 0;
    for (auto i : scene)
    {
        if (i.shader.program == shader.program)
        {
            shader_section_index = i.index;
        }
        c++;
    }

    if (shader_section_index == -1)
    {
        int id = scene.size();
        scene.push_back({shader, {}, id});
        shader_section_index = id;
    }

    auto m = getMaterial(mat);

    // Find the correct section, or create if it doesn't exist
    int mat_section_index = -1;
    c = 0;
    for (auto i : scene[shader_section_index].mats)
    {
        if (i.mat.mat_ref == mat)
        {
            mat_section_index = i.index;
        }
        c++;
    }

    if (mat_section_index == -1)
    {
        // Add a new mat section
        int id = scene.at(shader_section_index).mats.size();
        scene.at(shader_section_index)
            .mats.push_back(MatSection<MeshSection<ROCpu, ROGpu>>{{mat, m->uniforms.getRef(), m->shader}, {}, id});
        mat_section_index = id;
    }

    MatSection<MeshSection<ROCpu, ROGpu>>& ms = scene.at(shader_section_index).mats[mat_section_index];

    // Find Mesh Section
    int mesh_section_index = -1;
    c = 0;
    for (auto i : ms.meshes)
    {
        if (i.mesh == mesh)
        {
            mesh_section_index = i.index;
        }
        c++;
    }

    if (mesh_section_index == -1)
    {
        // Add a new mesh section
        // We have to create some OpenGL Objects for this one

        // Create buffer for renderobjects

        std::vector<GLTools::_VertexAttribSize> sizes = {};
        for (int i = 0; i < rows; i++)
        {
            sizes.push_back({sizeof(glm::vec4), 4});
        }

        // TODO: Maybe make this unique_ptr?
        GLTools::Buffer<ROGpu>* gpu_buffer = new GLTools::Buffer<ROGpu>(GLTools::Dynamic, sizes, {});

        int id = ms.meshes.size();
        auto v = new GLTools::VertexArray();
        ms.meshes.push_back(MeshSection<ROCpu, ROGpu>{mesh, v, {}, gpu_buffer, id});
        int index = ms.meshes.size() - 1;

        GLERRORCHECK();
        ms.meshes[index].vao->addBuffer(meshes[mesh]);
        GLERRORCHECK();
        ms.meshes[index].vao->addBuffer(gpu_buffer);
        GLERRORCHECK();
        ms.meshes[index].vao->addIndices(indices[mesh].second, indices[mesh].first);
        GLERRORCHECK();
        ms.meshes[index].vao->bind();
        GLERRORCHECK();

        mesh_section_index = id;
    }

    MeshSection<ROCpu, ROGpu>& mes = ms.meshes[mesh_section_index];

    // Add to lookup table
    Frender::LookupId lid =
        render_object_lookup.push(RenderIndex{shader_section_index, mat_section_index, mesh_section_index});

    // Add actual data to CPU and GPU
    int id = mes.cpu_info.size();
    cpu.index = id;
    cpu.lookup_index = lid;
    cpu.bounding_box = bounding_boxes[mes.mesh];
    cpu.bounding_box.transformBoundingBox(transform);
    mes.cpu_info.push_back(cpu);
    mes.gpu_buffer->pushBack(gpu);

    return {Unlit, this, lid};
    // return {Unlit, shader_section_index, mat_section_index, mesh_section_index, id, this};
}

template <typename ROCpu, typename ROGpu>
void Frender::Renderer::_destroyRenderObject(std::vector<ShaderSection<MatSection<MeshSection<ROCpu, ROGpu>>>>& scene,
                                             RenderObjectRef ro)
{
    if (ro.renderer == nullptr)
    {
        return;
    }

    auto loc = render_object_lookup.at(ro.id);

    // Work backwards
    MeshSection<ROCpu, ROGpu>& mes = scene[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section];

    // Find our item
    int found_index = -1;
    for (auto i : mes.cpu_info)
    {
        if (i.index == loc.index)
        {
            found_index = i.index;

            // Delete the item and the index
            mes.gpu_buffer->pop(i.index);
        }
        else if (found_index != -1)
        {
            // Update the index
            i.index = i.index - 1;

            // Update lookup table
            loc.index = i.index;
            render_object_lookup.set(ro.id, loc);
            // TODO: There is space between when the lookup table is changed and when the actual list is changed
        }
    }

    mes.cpu_info.erase(mes.cpu_info.begin() + found_index);

    // Now we check the Mesh Section...
    if (mes.cpu_info.size() < 1)
    {
        // Delete the mesh section
        mes.vao->destroy(); // Do I re-use VertexArrays?
        mes.gpu_buffer->destroy();
        // TODO: Garbage collection for meshes

        // Update the indexes of the rest of them
        auto mats = scene[loc.shader_section].mats[loc.mat_section];
        mats.meshes.erase(mats.meshes.begin() + loc.mesh_section);

        for (int i = loc.mesh_section; i < mats.meshes.size(); i++)
        {
            mats.meshes[i].index = mats.meshes[i].index - 1;

            // Update lookup table of all subelements
            // TODO: Maybe make this it's own function?
            for (auto ro : mats.meshes[i].cpu_info)
            {
                Frender::RenderIndex ro_loc = render_object_lookup.at(ro.lookup_index);
                ro_loc.mesh_section = mats.meshes[i].index;
                render_object_lookup.set(ro.lookup_index, ro_loc);
            }
        }

        // Now we check the Mat Section...
        if (mats.meshes.size() < 1)
        {
            // Delete the mat section
            // TODO: Garbage collection for mats

            // Update the indexes of the rest of them
            auto shs = &scene[loc.shader_section];
            shs->mats.erase(shs->mats.begin() + loc.mat_section);

            for (int i = loc.mat_section; i < shs->mats.size(); i++)
            {
                shs->mats[i].index = shs->mats[i].index - 1;

                // Update lookup table of all subelements
                // TODO: Maybe make this it's own function?
                for (auto& mesh : shs->mats[i].meshes)
                {
                    for (auto ro : mesh.cpu_info)
                    {
                        Frender::RenderIndex ro_loc = render_object_lookup.at(ro.lookup_index);
                        ro_loc.mat_section = shs->mats[i].index;
                        render_object_lookup.set(ro.lookup_index, ro_loc);
                    }
                }
            }

            // Now we check the Shader Section...
            if (shs->mats.size() < 1)
            {
                // Delete the shader section
                // TODO: Garbage collection for shader

                // Update the indexes of the rest of them
                scene.erase(scene.begin() + loc.shader_section);

                for (int i = loc.shader_section; i < scene.size(); i++)
                {
                    scene[i].index = scene[i].index - 1;

                    for (auto& mats : scene[i].mats)
                    {
                        for (auto& mesh : mats.meshes)
                        {
                            for (auto ro : mesh.cpu_info)
                            {
                                Frender::RenderIndex ro_loc = render_object_lookup.at(ro.lookup_index);
                                ro_loc.shader_section = scene[i].index;
                                render_object_lookup.set(ro.lookup_index, ro_loc);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Frender::Renderer::destroyRenderObject(RenderObjectRef ro)
{
    switch (ro.type)
    {
    case (Lit):
        _destroyRenderObject(scene_tree, ro);
        return;

    case (Unlit):
        _destroyRenderObject(funlit_scene_tree, ro);
        return;

    default:
        break;
    }
}

Frender::RenderObjectRef Frender::Renderer::duplicateRenderObject(RenderObjectRef ro)
{
    auto loc = render_object_lookup.at(ro.id);
    if (ro.type == Lit)
    {
        return createUnlitRenderObject(
            scene_tree[loc.shader_section].shader,
            scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].mesh,
            scene_tree[loc.shader_section].mats[loc.mat_section].mat.mat_ref,
            scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].cpu_info[loc.index].model);
    }
    else
    {
        return createUnlitRenderObject(
            funlit_scene_tree[loc.shader_section].shader,
            funlit_scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].mesh,
            funlit_scene_tree[loc.shader_section].mats[loc.mat_section].mat.mat_ref,
            funlit_scene_tree[loc.shader_section]
                .mats[loc.mat_section]
                .meshes[loc.mesh_section]
                .cpu_info[loc.index]
                .model);
    }
}

Frender::RenderObjectTraits Frender::Renderer::getRenderObjectTraits(RenderObjectRef ro)
{
    auto loc = render_object_lookup.at(ro.id);
    if (ro.type == Lit)
    {
        if (loc.shader_section >= scene_tree.size() || loc.mat_section >= scene_tree[loc.shader_section].mats.size() ||
            loc.mesh_section >=
                scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].cpu_info.size())
        {
            std::cout << "Very, very bad\n";
        }

        return {
            ro.type, scene_tree[loc.shader_section].shader,
            scene_tree[loc.shader_section].mats[loc.mat_section].mat.mat_ref,
            scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].mesh,
            scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].cpu_info[loc.index].model};
    }
    else
    {
        return {ro.type, funlit_scene_tree[loc.shader_section].shader,
                funlit_scene_tree[loc.shader_section].mats[loc.mat_section].mat.mat_ref,
                funlit_scene_tree[loc.shader_section].mats[loc.mat_section].meshes[loc.mesh_section].mesh,
                funlit_scene_tree[loc.shader_section]
                    .mats[loc.mat_section]
                    .meshes[loc.mesh_section]
                    .cpu_info[loc.index]
                    .model};
    }
}

Frender::LightRef Frender::Renderer::createPointLight(glm::vec3 position, glm::vec3 color, float radius)
{
    radius *= 1.5;
    auto m = glm::scale(glm::translate(glm::mat4(), position), glm::vec3(radius));

    // TODO: Remove the ptrs here
    auto light_id = new int32_t(light_index);
    point_lights.push_back({color, position, radius, m, light_id});

#ifndef FLUX_NO_DEFFERED
    // New method: Add to buffer
    point_light_buffer.pushBack({color, position, radius, m});
#endif

    light_index++;
    return Frender::LightRef(this, Point, new int32_t(point_lights.size() - 1));
}

Frender::LightRef Frender::Renderer::createDirectionalLight(glm::vec3 color, glm::vec3 direction)
{
    auto light_id = new int32_t(light_index);
    directional_lights.push_back({color, direction, light_id});

    // Add to Extrema list
    // addExtrema({Extrema::Minima, Extrema::Light, glm::vec3(-1000000), light_id, nullptr, {}});
    // addExtrema({Extrema::Maxima, Extrema::Light, glm::vec3(1000000), light_id, nullptr, {}});

    // Add to light buffer
    // light_buffer.setArray("light_pos_dir_rad", light_index, glm::vec4(direction.x, direction.y, direction.z, 0));
    // light_buffer.setArray("light_color_type", light_index, glm::vec4(color.x, color.y, color.z, 1));

    light_index++;
    return Frender::LightRef(this, Directional, new int32_t(directional_lights.size() - 1));
}

glm::mat4 Frender::RenderObjectRef::getTransform()
{
    return renderer->_getRenderObject(id, type).model;
}

void Frender::RenderObjectRef::setTransform(glm::mat4 t)
{
    renderer->_getRenderObject(id, type).model = t;
}

Frender::RenderObjectRef Frender::RenderObjectRef::duplicate()
{
    return renderer->duplicateRenderObject(*this);
}

glm::vec3 Frender::Renderer::getLightPosition(LightRef light)
{
    if (light.type == Directional)
    {
        return glm::vec3(0, 0, 0);
    }

    // Assume it's a point light
    return point_lights[*light.index].position;
}

void Frender::Renderer::setLightPosition(LightRef light, glm::vec3 position)
{
    if (light.type == Directional)
    {
        return;
    }

    // removeExtrema(point_lights[*light.index].minima);
    // removeExtrema(point_lights[*light.index].maxima);

    // Change the position
    point_lights[*light.index].position = position;

    // Re-add the extrema
    float radius = point_lights[*light.index].radius;
    auto light_id = point_lights[*light.index].light_id;
    auto m = glm::scale(glm::translate(glm::mat4(), position), glm::vec3(radius));
    // point_lights[*light.index].minima =
    // addExtrema({Extrema::Minima, Extrema::Light, position - glm::vec3(radius), light_id, nullptr, {}});
    // point_lights[*light.index].maxima =
    // addExtrema({Extrema::Maxima, Extrema::Light, position + glm::vec3(radius), light_id, nullptr, {}});

#ifndef FLUX_NO_DEFERED
    point_light_buffer.set(*light.index, {point_lights[*light.index].color, position, radius, m});
#endif

    glm::vec3 color = point_lights[*light.index].color;

    // Add to light buffer
    // light_buffer.setArray("light_pos_dir_rad", *light_id, glm::vec4(position.x, position.y, position.z, radius));
    // light_buffer.setArray("light_color_type", *light_id, glm::vec4(color.x, color.y, color.z, 0));
}

void Frender::Renderer::destroyLight(LightRef light)
{
    if (light.type == Point)
    {
        // Remove from sweep and prune
        // removeExtrema(point_lights[*light.index].minima);
        // removeExtrema(point_lights[*light.index].maxima);

        // Remove from point_lights
        point_lights.erase(point_lights.begin() + *light.index);

        // Remove from light buffer and point_light_buffer
#ifndef FLUX_NO_DEFERED
        point_light_buffer.pop(*light.index);
#endif

        // Update the indexes of every other light
        for (int i = *light.index; i < point_lights.size(); i++)
        {
            // Update index
            *point_lights[i].light_id = i;

            // Update it in VRAM
            // light_buffer.setArray("light_pos_dir_rad", i,
            // glm::vec4(point_lights[i].position.x, point_lights[i].position.y,
            // point_lights[i].position.z, point_lights[i].radius));
            // light_buffer.setArray(
            // "light_color_type", i,
            // glm::vec4(point_lights[i].color.x, point_lights[i].color.y, point_lights[i].color.z, 0));
        }

        // Set memory to -1 so that if it's accessed before it's reused it crashes there and then
        *light.index = -1;
        delete light.index;
    }
    else
    {
        // Remove from sweep and prune
        // removeExtrema(directional_lights[*light.index].minima);
        // removeExtrema(directional_lights[*light.index].maxima);

        // Remove from directional_lights
        directional_lights.erase(directional_lights.begin() + *light.index);

        // Update the indexes of every other light
        for (int i = *light.index; i < directional_lights.size(); i++)
        {
            // Update index
            *directional_lights[i].light_id = i;

            // Update it in VRAM
            // light_buffer.setArray("light_pos_dir_rad", i,
            // glm::vec4(directional_lights[i].direction.x, directional_lights[i].direction.y,
            // directional_lights[i].direction.z, 0));
            // light_buffer.setArray("light_color_type", i,
            // glm::vec4(directional_lights[i].color.x, directional_lights[i].color.y,
            // directional_lights[i].color.z, 1));
        }

        // Set memory to -1 so that if it's accessed before it's reused it crashes there and then
        *light.index = -1;
        delete light.index;
    }
}

glm::vec3 Frender::Renderer::getLightDirection(LightRef light)
{
    if (light.type != Directional)
    {
        return glm::vec3(0, 0, 0);
    }

    // Assume it's a directional light
    return directional_lights[*light.index].direction;
}

void Frender::Renderer::setLightDirection(LightRef light, glm::vec3 direction)
{
    if (light.type != Directional)
    {
        return;
    }

    directional_lights[*light.index].direction = direction;
    // light_buffer.setArray("light_pos_dir_rad", *light.index, glm::vec4(direction.x, direction.y, direction.z, 0));
}

void Frender::Renderer::setSkybox(int width, int height, float* data)
{
    auto tx = Texture(width, height, data);
    // Normal one for skybox
    sky_cubemap = GLTools::equirectangularToCubemap(equiToCubemap_shader, tx);

    // Convoluted one for irradience
    irradiance_cubemap = GLTools::equirectangularToCubemap(equiToCubemap_convolution_shader, tx);

    // Convoluted one for prefilter
    prefilter_cubemap = GLTools::equirectangularToCubemap(equiToCubemap_prefilter_shader, tx, true, 128, 128);

    // Add it to all the textures
    alight_tx.set("irradiance_map", irradiance_cubemap, GLTools::CUBEMAP_PX);
    alight_tx.set("prefilter_map", prefilter_cubemap, GLTools::CUBEMAP_PX);

    tx.destroy();
    skybox_textures.set("skybox", sky_cubemap, GLTools::CUBEMAP_PX);

    has_skybox = true;
}
// TODO: Free memory allocated by the buffers, and generally clean up better
