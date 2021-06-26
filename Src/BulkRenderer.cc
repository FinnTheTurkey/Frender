#include "Frender/Frender.hh"
#include <glad/glad.h>

void Frender::Renderer::bulkRender()
{
    stage1_bulk_shader.enable();
    for (auto i : render_objects)
    {
        i.mesh.enable();

        // Enable material UBO
        i.mat.uniforms.enable();
        
        // Set important uniforms
        i.mat.shader.setUniforms(projection * inv_camera * i.transform, i.transform);

        // Enable textures
        // Cache miss :(
        i.mat.textures->enable();

        glDrawElements(GL_TRIANGLES, i.mesh.num_indices, GL_UNSIGNED_INT, 0);
    }
}