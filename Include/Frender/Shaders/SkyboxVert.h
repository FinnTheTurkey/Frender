// Auto generated file.
static const char SkyboxVertSrc[] = "#version 330 core\n\
#define GLSLIFY 1\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 normals;\n\
layout (location = 2) in vec2 tx_coords;\n\
\n\
out vec3 tex_coords;\n\
\n\
uniform mat4 vp;\n\
\n\
void main()\n\
{\n\
    tex_coords = aPos;\n\
    vec4 pos = vp * vec4(aPos, 1.0);\n\
    gl_Position = pos.xyww;\n\
}";
