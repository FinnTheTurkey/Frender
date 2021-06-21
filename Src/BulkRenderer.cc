#include "Frender/Frender.hh"
#include <glad/glad.h>

void Frender::Renderer::bulkRender()
{
    stage1_bulk_shader.enable();
    for (auto i : render_objects)
    {
        i.mesh.enable();
        glDrawElements(GL_TRIANGLES, i.mesh.num_indices, GL_UNSIGNED_INT, 0);
    }
}