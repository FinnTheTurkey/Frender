// Auto generated file.
static const char UnlitFragSrc[] = "#version 330 core\n\
#define GLSLIFY 1\n\
layout (location = 0) out vec4 FragColor;\n\
layout (location = 1) out vec4 BrightColor;\n\
\n\
in vec2 tex_coords;\n\
in vec3 normal;\n\
in vec3 world_pos;\n\
\n\
in mat3 tbn;\n\
\n\
layout (std140) uniform Material \n\
{\n\
    vec3 color;\n\
    float emmisive;\n\
    int has_diffuse_map;\n\
};\n\
\n\
uniform sampler2D diffuse_map;\n\
\n\
void main()\n\
{\n\
    if (has_diffuse_map == 1)\n\
    {\n\
        vec4 tx = texture(diffuse_map, tex_coords);\n\
        FragColor = vec4(tx.xyz, 1);\n\
\n\
        if (tx.w == 0)\n\
        {\n\
            discard;\n\
        }\n\
    }\n\
    else\n\
    {\n\
        FragColor = vec4(color, 1);\n\
    }\n\
\n\
    if (emmisive > 1)\n\
    {\n\
        BrightColor = vec4(FragColor.xyz * emmisive, 1);\n\
    }\n\
    else\n\
    {\n\
        BrightColor = vec4(0, 0, 0, 1);\n\
    }\n\
}";
