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

    auto vp = projection * inv_camera;
    auto fcvp = vp;

    // Create frustum planes - this probably won't work
    // Also, try not to do it _every frame_
    for (int i = 4; i--; ) frustum_planes[0][i]    = fcvp[i][3] + fcvp[i][0];
    for (int i = 4; i--; ) frustum_planes[1][i]    = fcvp[i][3] - fcvp[i][0]; 
    for (int i = 4; i--; ) frustum_planes[2][i]    = fcvp[i][3] + fcvp[i][1];
    for (int i = 4; i--; ) frustum_planes[3][i]    = fcvp[i][3] - fcvp[i][1];
    for (int i = 4; i--; ) frustum_planes[4][i]    = fcvp[i][3] + fcvp[i][2];
    for (int i = 4; i--; ) frustum_planes[5][i]    = fcvp[i][3] - fcvp[i][2];

    stage1_bulk_shader.enable();
    for (auto mat : scene_tree)
    {
        mat.mat.uniforms.enable();
        getMaterial(mat.mat.mat_ref)->textures.enable();

        for (auto mesh : mat.meshes)
        {
            // Update transforms
            int to_draw = 0;
            int max = mesh.gpu_buffer->size()-1;
            for (auto ro : mesh.cpu_info)
            {
                // Frustum culling
                // ro.bounding_box.transformBoundingBox(ro.model);

                // TODO: Fix frustum culling
                // if (!frustumCull(ro.bounding_box.min_pos, ro.bounding_box.max_pos))
                if (false)
                {
                    // No point is in the view frustum, so we can safely cull
                    // Add to the end of the buffer
                    mesh.gpu_buffer->set(max, {glm::mat4(), ro.model});
                    max --;
                }
                else
                {
                    // We do have to draw it
                    auto mvp = vp * ro.model;
                    mesh.gpu_buffer->set(to_draw, {mvp, ro.model});
                    to_draw++;
                }
            }

            if (to_draw > 0)
            {
                // Upload to GPU
                mesh.gpu_buffer->apply();

                // Render
                mesh.vao->enable();
                mesh.vao->draw(to_draw); // Only render elements added to the front
            }
        }
    }

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
    // New method: Draw instanced
    // Update all the uniforms
    int good = 0, bad = point_light_buffer.size();
    for (int i = 0; i < point_light_buffer.size(); i++)
    {
        auto v = point_light_buffer.get(i);
        v.transform = vp * glm::scale(glm::translate(glm::mat4(), v.position), glm::vec3(v.radius));
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

bool Frender::Renderer::frustumCull(glm::vec3 min, glm::vec3 max)
{
    // Copy and pasted from a sketchy site
    // bool inside = true; //test all 6 frustum planes 
    for (int i = 0; i<6; i++) 
    {
        //pick closest point to plane and check if it behind the plane
        //if yes - object outside frustum 
        float d = glm::max(min.x * frustum_planes[i].x, max.x * frustum_planes[i].x) + glm::max(min.y * frustum_planes[i].y, max.y * frustum_planes[i].y) + glm::max(min.z * frustum_planes[i].z, max.z * frustum_planes[i].z) + frustum_planes[i].w;
        // inside &= d > 0;
        if (d < -1)
        {
            return false; //with flag works faster
        }
    } 
    return true;
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