#include "Frender/Frender.hh"
#include "Frender/GLTools.hh"
#include "glm/gtc/matrix_transform.hpp"
#include <glad/glad.h>
#include <iostream>

void Frender::Renderer::bulkRender()
{
    // Stage 1: Geometry pass
    stage2_fbo.enable();
    GLERRORCHECK();
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClear(GL_DEPTH_BUFFER_BIT);
    GLERRORCHECK();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    GLERRORCHECK();

    auto mv = projection * inv_camera;

    stage1_bulk_shader.enable();
    for (auto mat : scene_tree)
    {
        mat.mat.uniforms.enable();
        getMaterial(mat.mat.mat_ref)->textures.enable();

        for (auto mesh : mat.meshes)
        {
            // Update transforms
            int i = 0;
            for (auto ro : mesh.cpu_info)
            {
                mesh.gpu_buffer->set(i, {mv * ro.model, ro.model});
                i++;
            }

            // Upload to GPU
            mesh.gpu_buffer->apply();

            // Render
            mesh.vao.enable();
            mesh.vao.draw(mesh.cpu_info.size());
        }
    }
    // for (auto i : render_objects)
    // {
    //     i.mesh.enable();
    //     GLERRORCHECK();

    //     // Enable material UBO
    //     i.mat.uniforms.enable();
    //     GLERRORCHECK();
        
    //     // Set important uniforms
    //     i.mat.shader.setUniforms(mv * i.transform, i.transform);
    //     GLERRORCHECK();

    //     // Enable textures
    //     // Cache miss :(
    //     getMaterial(i.mat.mat_ref)->textures.enable();
    //     GLERRORCHECK();

    //     glDrawElements(GL_TRIANGLES, i.mesh.num_indices, GL_UNSIGNED_INT, 0);
    //     GLERRORCHECK();
    // }

    // stage2_fbo.disable();

    // Stage 2: Lighting pass and forward rendering
    // Copy over depth buffer and enable stage3_fbo
    GLTools::transferDepthBuffer(&stage2_fbo, &stage3_fbo, width, height);
    // stage3_fbo.enable();

    // Setup screen
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    
    // glDisable(GL_CULL_FACE);
    // glEnable(GL_C)
    // glDisable(GL_DEPTH_TEST);
    GLERRORCHECK();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    // glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Directional loghts
    stage2_dlight_shader.enable();
    stage2_texd.enable();

    // Set important uniforms
    stage2_dlight_shader.setUniform(dlight_uniforms.width, width);
    stage2_dlight_shader.setUniform(dlight_uniforms.height, height);
    stage2_dlight_shader.setUniform(dlight_uniforms.cam_pos, camera * glm::vec4(0, 0, 0, 1));

    plane.enable();

    for (auto i : directional_lights)
    {
        // Light pos is actually direction for directinal lights
        stage2_dlight_shader.setUniform(dlight_uniforms.light_pos, i.direction);
        GLERRORCHECK();
        stage2_dlight_shader.setUniform(dlight_uniforms.color, i.color);

        // No need for transformation matrices since we're just drawing a plane
        glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
    }

    // Use depth testing and depth buffer for lights that don't effect everything
    // GLTools::transferDepthBuffer(&stage2_fbo, &stage3_fbo, width, height);
    // glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glCullFace(GL_FRONT); // Only draw backfaces for best effect

    // Enable all nessesary shaders and meshes
    stage2_light_shader.enable();
    stage2_tex.enable();
    // light_sphere.enable();

    // Set important uniforms
    stage2_light_shader.setUniform(light_uniforms.width, width);
    stage2_light_shader.setUniform(light_uniforms.height, height);
    stage2_light_shader.setUniform(light_uniforms.cam_pos, camera * glm::vec4(0, 0, 0, 1));
    
    // Run lighting pass
    // for (auto i : point_lights)
    // {
    //     // Set important uniforms
    //     stage2_light_shader.setUniform(light_uniforms.color, i.color);
    //     stage2_light_shader.setUniform(light_uniforms.light_pos, i.position);
    //     stage2_light_shader.setUniform(light_uniforms.radius, i.radius);

    //     // Create transformation matrix
    //     auto m = glm::scale(glm::translate(glm::mat4(), i.position), glm::vec3(i.radius));
    //     stage2_light_shader.setUniforms(projection * inv_camera * m, m);

    //     // Render
    //     glDrawElements(GL_TRIANGLES, light_sphere.num_indices, GL_UNSIGNED_INT, 0);
    // }

    // New method: Draw instanced
    // Update all the uniforms
    for (int i = 0; i < point_light_buffer.size(); i++)
    {
        auto v = point_light_buffer.get(i);
        v.transform = mv * glm::scale(glm::translate(glm::mat4(), v.position), glm::vec3(v.radius));
        point_light_buffer.set(i, v);
    }

    // Upload changes to gpu
    point_light_buffer.apply();
    GLERRORCHECK();

    light_sphere_vao.enable();
    GLERRORCHECK();
    light_sphere_vao.draw(point_light_buffer.size());
    GLERRORCHECK();

    // Debug
    // auto bi = point_light_buffer._getBufferInfo();
    // glBindBuffer(GL_ARRAY_BUFFER, bi.handle);
    // int output;
    // glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &output);

    // std::cout << "Buffer size: " << output << "\n";

    stage3_fbo.disable();
    // glEnable(GL_CULL_FACE);
    // glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    // glCullFace(GL_FRONT);

    // Stage 2.99: Bloom
    plane.enable();
    GLERRORCHECK();

    if (bloom_res_scale != 0)
    {
        processBloom();
    }

    // Stage 3: Post-processing and HUD elements
    stage3_shader.enable();
    GLERRORCHECK();
    stage3_tex.enable();
    GLERRORCHECK();
    
    // Set bloom parameters
    stage3_shader.setUniform(bloom_exposure_loc, bloom_blur_amount == 0 ? 0 : bloom_exposure);

    glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
    GLERRORCHECK();
}

void Frender::Renderer::processBloom()
{
    // Plane should already be enabled
    bloom_shader.enable();
    GLERRORCHECK();

    // Put the initial brightness into the bloom loop
    bool first = true;
    bloom_tex1.set(0, stage3_fbo.getTexture()[1]);

    // glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // Don't do this
    bloom_fbo1.enable();
    bloom_shader.enable();
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bloom_fbo2.enable();
    bloom_shader.enable();
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool horizontal = true;
    for (int i = 0; i < bloom_blur_amount; i++)
    {
        // Do horizontal pass
        bloom_tex1.enable();
        bloom_fbo1.enable();
        // glClearColor(1, 0, 0, 1);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloom_shader.setUniform(bloom_horizontal_loc, true);

        glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
        GLERRORCHECK();

        if (first)
        {
            first = false;
            bloom_tex1.set(0, bloom_fbo2.getTexture()[0]);
        }

        // Do vertical pass
        bloom_tex2.enable();
        bloom_fbo2.enable();
        // glClearColor(1, 0, 0, 1);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloom_shader.setUniform(bloom_horizontal_loc, false);

        glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
        GLERRORCHECK();
    }

    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    bloom_fbo2.disable();
}