// Auto generated file.
static const char BulkStage1FragSrc[] = "#version 330 core\n\
layout (location = 0) out vec4 ColorRoughness;\n\
layout (location = 1) out vec4 NormalMetallic;\n\
layout (location = 2) out vec3 position;\n\
\n\
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
    float roughness;\n\
    float metalness;\n\
    int has_diffuse_map;\n\
    int has_normal_map;\n\
    int has_roughness_map;\n\
    int has_metal_map;\n\
};\n\
\n\
uniform sampler2D diffuse_map;\n\
uniform sampler2D metal_map;\n\
uniform sampler2D normal_map;\n\
uniform sampler2D roughness_map;\n\
\n\
// uniform vec3 cam_pos;\n\
\n\
void main()\n\
{\n\
    float rness = roughness;\n\
    float mness = metalness;\n\
\n\
    if (has_roughness_map == 1)\n\
    {\n\
        rness = texture(roughness_map, tex_coords).x;\n\
    }\n\
    if (has_metal_map == 1)\n\
    {\n\
        mness = texture(metal_map, tex_coords).x;\n\
    }\n\
\n\
    rness = clamp(rness, 0.01, 0.99);\n\
    mness = clamp(mness, 0.01, 0.99);\n\
\n\
    vec3 N = normal.xyz;\n\
    if (has_normal_map == 1 /*&& distance(world_pos, cam_pos) < 10*/)\n\
    {\n\
        N = texture(normal_map, tex_coords).xyz;\n\
\n\
        // Make the normal map work properly\n\
        N = normalize(N * 2.0 - 1.0);\n\
        N = normalize(tbn * N);\n\
    }\n\
\n\
    NormalMetallic = vec4(N, mness);\n\
\n\
    if (has_diffuse_map == 1)\n\
    {\n\
        vec4 tx = texture(diffuse_map, tex_coords);\n\
        ColorRoughness = vec4(tx.xyz, rness);\n\
\n\
        if (tx.w == 0)\n\
        {\n\
            discard;\n\
        }\n\
    }\n\
    else\n\
    {\n\
        ColorRoughness = vec4(color, rness);\n\
    }\n\
\n\
    position = world_pos;\n\
}";
