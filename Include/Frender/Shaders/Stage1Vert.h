// Auto generated file.
static const char BulkStage1VertSrc[] = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
\n\
uniform mat4 mvp;\n\
uniform mat4 model;\n\
\n\
void main()\n\
{\n\
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
}";
