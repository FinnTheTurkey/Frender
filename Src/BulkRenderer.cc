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

    stage1_bulk_shader.enable();
    for (auto i : render_objects)
    {
        i.mesh.enable();
        GLERRORCHECK();

        // Enable material UBO
        i.mat.uniforms.enable();
        GLERRORCHECK();
        
        // Set important uniforms
        i.mat.shader.setUniforms(projection * inv_camera * i.transform, i.transform);
        GLERRORCHECK();

        // Enable textures
        // Cache miss :(
        getMaterial(i.mat.mat_ref)->textures.enable();
        GLERRORCHECK();

        glDrawElements(GL_TRIANGLES, i.mesh.num_indices, GL_UNSIGNED_INT, 0);
        GLERRORCHECK();
    }

    // stage2_fbo.disable();

    // Stage 2: Lighting pass and forward rendering
    // Copy over depth buffer and enable stage3_fbo
    // GLTools::transferDepthBuffer(&stage2_fbo, &stage3_fbo, width, height);
    stage3_fbo.enable();

    // Setup screen
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glCullFace(GL_FRONT); // Only draw backfaces for best effect
    glDisable(GL_CULL_FACE);
    GLERRORCHECK();

    // Enable all nessesairy shaders and meshes
    stage2_light_shader.enable();
    stage2_tex.enable();
    light_sphere.enable();

    // Set important uniforms
    stage2_light_shader.setUniform(light_uniforms.width, width);
    stage2_light_shader.setUniform(light_uniforms.height, height);
    stage2_light_shader.setUniform(light_uniforms.cam_pos, camera * glm::vec4(0, 0, 0, 1));
    
    // Run lighting pass
    for (auto i : point_lights)
    {
        // Set important uniforms
        stage2_light_shader.setUniform(light_uniforms.color, i.color);
        stage2_light_shader.setUniform(light_uniforms.light_pos, i.position);
        stage2_light_shader.setUniform(light_uniforms.radius, i.radius);

        // Create transformation matrix
        auto m = glm::scale(glm::translate(glm::mat4(), i.position), glm::vec3(i.radius));
        stage2_light_shader.setUniforms(projection * inv_camera * m, m);

        // Render
        glDrawElements(GL_TRIANGLES, light_sphere.num_indices, GL_UNSIGNED_INT, 0);
    }

    stage3_fbo.disable();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // glCullFace(GL_FRONT);

    // Stage 3: Post-processing and HUD elements
    plane.enable();
    GLERRORCHECK();
    stage3_shader.enable();
    GLERRORCHECK();
    stage3_tex.enable();
    GLERRORCHECK();

    glDrawElements(GL_TRIANGLES, plane.num_indices, GL_UNSIGNED_INT, 0);
    GLERRORCHECK();
}