// Auto generated file.
static const char EquiToCubemapVertSrc[] = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
\n\
out vec3 localPos;\n\
\n\
uniform mat4 projection;\n\
uniform mat4 view;\n\
\n\
void main()\n\
{\n\
    localPos = aPos;\n\
    gl_Position =  projection * view * vec4(localPos, 1.0);\n\
}";
