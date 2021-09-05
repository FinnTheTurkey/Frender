#include "Frender/Frender.hh"
#include "Frender/GLTools.hh"
#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>
#include <glad/glad.h>
#include <iostream>

void Frender::Renderer::bulkRender()
{

    glViewport(0, 0, width, height);
    auto vp = projection * inv_camera;
#ifndef FRENDER_NO_DEFERED
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
    // stage1_bulk_shader.setUniform(stage1_bulk_shader.getUniformLocation("cam_pos"), camera * glm::vec4(0, 0, 0, 1));

    geometryPass(vp);

    // stage2_fbo.disable();

    // Stage 2: Lighting pass and forward rendering
    // Copy over depth buffer and enable stage3_fbo
    GLTools::transferDepthBuffer(&stage2_fbo, &stage3_fbo, width, height);
    // stage3_fbo.enable();

    // Setup screen
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

    // Disable writing to the depth buffer
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

    // Ambient/cubemap lighting
    stage2_alight_shader.enable();
    alight_tx.enable();

    // Set important uniforms
    stage2_alight_shader.setUniform(alight_uniforms.width, width);
    stage2_alight_shader.setUniform(alight_uniforms.height, height);
    stage2_alight_shader.setUniform(alight_uniforms.cam_pos, camera * glm::vec4(0, 0, 0, 1));

    glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);

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

    // Render unlit objects
    glDepthMask(GL_TRUE);

    // stage3_fbo.disable();
    // glEnable(GL_CULL_FACE);
    // glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    // glCullFace(GL_FRONT);
#else
    stage3_fbo.enable();
    glClearColor(0, 1, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

    // Unlit/Forward rendered objects
    unlitRender(vp);
    GLERRORCHECK();

    // Lit forward objects
    // Calculate light intersections
    calculateLighting(vp);
    GLERRORCHECK();
    litRender(vp);
    GLERRORCHECK();

    // Render skybox
    if (has_skybox)
    {
        // glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);

        skybox_shader.enable();

        // Remove transformations
        glm::mat4 special_vp = projection * glm::mat4(glm::mat3(inv_camera));
        skybox_shader.setUniform(skybox_vp_loc, special_vp);

        skybox_textures.enable();
        cube.enable();
        glDrawElements(GL_TRIANGLES, cube.num_indices, GL_UNSIGNED_INT, 0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
        // glEnable(GL_DEPTH_TEST);
    }

    stage3_fbo.disable();

    GLERRORCHECK();
    // Stage 2.99: Bloom
    plane.enable();
    GLERRORCHECK();

    if (bloom_res_scale != 0)
    {
        processBloom();
    }

    // Stage 3: Post-processing and HUD elements
    if (use_fxaa)
    {
        stage3fxaa_shader.enable();
    }
    else
    {
        stage3_shader.enable();
    }
    GLERRORCHECK();
    stage3_tex.enable();
    GLERRORCHECK();
    
    // Set bloom parameters
    stage3_shader.setUniform(use_fxaa ? bloom_exposure_loc_fxaa : bloom_exposure_loc, bloom_blur_amount == 0 ? 0 : bloom_exposure);

    // glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
    // glEnable(GL_DEPTH_TEST);
    GLERRORCHECK();
}

void Frender::Renderer::geometryPass(glm::mat4 vp)
{
    for (auto shdr : scene_tree)
    {
        for (auto mat : shdr.mats)
        {
            mat.mat.uniforms.enable(0);
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
    }
}

void Frender::Renderer::unlitRender(glm::mat4 vp)
{
    for (auto shdr : funlit_scene_tree)
    {
        shdr.shader.enable();
        for (auto mat : shdr.mats)
        {
            mat.mat.uniforms.enable(0);
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
    }
}


void Frender::Renderer::litRender(glm::mat4 vp)
{
    for (auto shdr : flit_scene_tree)
    {
        shdr.shader.enable();
        light_buffer.enable(1);
        // TODO: Make shader independant
        shdr.shader.setUniform(lit_uniforms.cam_pos, camera * glm::vec4(0, 0, 0, 1));
        GLERRORCHECK();
        for (auto mat : shdr.mats)
        {
            mat.mat.uniforms.enable(0);
            GLERRORCHECK();
            
            getMaterial(mat.mat.mat_ref)->textures.set(0, irradiance_cubemap);
            getMaterial(mat.mat.mat_ref)->textures.set(1, prefilter_cubemap);
            getMaterial(mat.mat.mat_ref)->textures.enable();

            GLERRORCHECK();

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
                    // if (false)
                    // {
                    //     // No point is in the view frustum, so we can safely cull
                    //     // Add to the end of the buffer
                    //     // auto gput = mesh.gpu_buffer->get(max);
                    //     // gput.model = ro.model;
                    //     mesh.gpu_buffer->set(max, {glm::mat4(), ro.model});
                    //     max --;
                    // }
                    // else
                    // {
                        // We do have to draw it
                        auto mvp = vp * ro.model;

                        // We have to keep the light calculations intact
                        auto gput = mesh.gpu_buffer->get(to_draw);
                        gput.model = ro.model;
                        gput.mvp = mvp;
                        mesh.gpu_buffer->set(to_draw, gput);
                        GLERRORCHECK();
                        to_draw++;
                    // }
                }

                if (to_draw > 0)
                {
                    // Upload to GPU
                    mesh.gpu_buffer->apply();
                    GLERRORCHECK();

                    // Render
                    mesh.vao->enable();
                    GLERRORCHECK();
                    mesh.vao->draw(to_draw); // Only render elements added to the front
                    GLERRORCHECK();
                }
            }
        }
    }
}

void Frender::Renderer::calculateLighting(glm::mat4 vp)
{
    // TODO: Deal with shiz moving
    std::vector<int> active_lights;
    bool first = true;
    int axis = 0;
    for (auto i : broad_phase)
    {
        for (auto e : i)
        {
            if (e.obj == Extrema::Light)
            {
                if (e.et == Extrema::Minima)
                {
                    active_lights.push_back(e.light);
                }
                else
                {
                    active_lights.erase(std::find(active_lights.begin(), active_lights.end(), e.light));
                }
            }
            else
            {
                ROInfoLit* item = &flit_scene_tree[e.shader_section].mats[e.mat_section].meshes[e.mesh_section].cpu_info[e.index];

                // Add all the lights colliding on the X axis
                if (first)
                {
                    if (e.et == Extrema::Minima)
                    {
                        for (int n = 0; n < 8; n++)
                        {
                            if (n < active_lights.size())
                            {
                                item->complete_lights[n] = active_lights[n];
                            }
                            else
                            {
                                item->complete_lights[n] = -1;
                            }
                        }
                    }
                    else
                    {
                        // Make sure we don't add anything twice
                        for (auto n : active_lights)
                        {
                            if (std::find(item->complete_lights.begin(), item->complete_lights.end(), n) == item->complete_lights.end())
                            {
                                for (int nn = 0; nn < 8; nn++)
                                {
                                    if (item->complete_lights[nn] == -1)
                                    {
                                        item->complete_lights[nn] = n;
                                        break;
                                    }
                                }
                            }
                        }

                        item->lights = item->complete_lights;
                    }
                }
                else
                {
                    // Lights have to be overlapping on _all_ axis to be overlaping,
                    // So from now on we only remove

                    if (e.et == Extrema::Minima)
                    {
                        for (auto& n : item->lights)
                        {
                            if (std::find(active_lights.begin(), active_lights.end(), n) == active_lights.end())
                            {
                                // Remove it from lights
                                n = -1; // This may not work
                            }
                        }
                    }
                    else
                    {
                        for (auto& n : item->lights)
                        {
                            if (std::find(active_lights.begin(), active_lights.end(), n) == active_lights.end())
                            {
                                // Remove it from lights
                                n = -1; // This may not work
                            }
                        }

                        // Find lights that were accidentally removed in the Minima
                        for (auto nn : active_lights)
                        {
                            // If it's active, not in lights but _is_ in complete_lights, then it needs to be put back
                            if (std::find(item->lights.begin(), item->lights.end(), nn) == item->lights.end()
                                && std::find(item->complete_lights.begin(), item->complete_lights.end(), nn) != item->complete_lights.end())
                            {
                                // Re-add it
                                for (int n = 0; n < 8; n++)
                                {
                                    if (item->complete_lights[n] == -1)
                                    {
                                        item->complete_lights[n] = n;
                                        break;
                                    }
                                }
                            }
                        }

                        item->complete_lights = item->lights;

                        if (axis == 2)
                        {
                            // All light collisions are calculated
                            // So we can put it in GPUInfo

                            Frender::ROInfoGPULit gpu_item = flit_scene_tree[e.shader_section].mats[e.mat_section].meshes[e.mesh_section].gpu_buffer->get(e.index);
                            gpu_item.lights1[0] = item->complete_lights[0];
                            gpu_item.lights1[1] = item->complete_lights[1];
                            gpu_item.lights1[2] = item->complete_lights[2];
                            gpu_item.lights1[3] = item->complete_lights[3];

                            gpu_item.lights2[0] = item->complete_lights[4];
                            gpu_item.lights2[1] = item->complete_lights[5];
                            gpu_item.lights2[2] = item->complete_lights[6];
                            gpu_item.lights2[3] = item->complete_lights[7];

                            flit_scene_tree[e.shader_section].mats[e.mat_section].meshes[e.mesh_section].gpu_buffer->set(e.index, gpu_item);
                        }
                    }
                }
            }
        }

        first = false;
        axis ++;
    }
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