// Auto generated file.
static const char Stage2VertSrc[] = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 normals;\n\
layout (location = 2) in vec2 tex_coord;\n\
layout (location = 3) in vec3 tang;\n\
layout (location = 4) in vec3 bitang;\n\
out vec2 tex_coords;\n\
\n\
uniform mat4 mvp;\n\
uniform mat4 model;\n\
\n\
void main()\n\
{\n\
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
    tex_coords = tex_coord;\n\
}";
