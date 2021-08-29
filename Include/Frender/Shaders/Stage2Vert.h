// Auto generated file.
static const char Stage2VertSrc[] = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 normals;\n\
layout (location = 2) in vec2 tex_coord;\n\
layout (location = 3) in vec3 tang;\n\
layout (location = 4) in vec3 bitang;\n\
\n\
layout (location = 5) in vec3 light_color_arg;\n\
layout (location = 6) in vec3 light_pos_arg;\n\
layout (location = 7)  in float radius_arg;\n\
layout (location = 8) in mat4 mvp;\n\
\n\
out vec2 tex_coords;\n\
\n\
out vec3 light_color;\n\
out vec3 light_pos;\n\
out float radius;\n\
\n\
// uniform mat4 mvp;\n\
// uniform mat4 model;\n\
\n\
void main()\n\
{\n\
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
    tex_coords = tex_coord;\n\
\n\
    light_color = light_color_arg;\n\
    light_pos = light_pos_arg;\n\
    radius = radius_arg;\n\
}";
