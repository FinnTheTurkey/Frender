// Auto generated file.
static const char BulkStage1FragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
\n\
layout (std140) uniform Material \n\
{\n\
    vec3 color;\n\
};\n\
\n\
void main()\n\
{\n\
    FragColor = vec4(color, 1.0f);\n\
} ";
