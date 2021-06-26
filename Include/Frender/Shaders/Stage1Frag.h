// Auto generated file.
static const char BulkStage1FragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 tex_coords;\n\
\n\
layout (std140) uniform Material \n\
{\n\
    vec3 color;\n\
};\n\
\n\
uniform sampler2D tex;\n\
\n\
void main()\n\
{\n\
    // FragColor = vec4(color, 1.0f);\n\
    FragColor = texture(tex, tex_coords);\n\
} ";
