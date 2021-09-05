#version 330 core
layout (location = 0) out vec4 ColorRoughness;
layout (location = 1) out vec4 NormalMetallic;
layout (location = 2) out vec3 position;


in vec2 tex_coords;
in vec3 normal;
in vec3 world_pos;

in mat3 tbn;

layout (std140) uniform Material 
{
    vec3 color;
    float roughness;
    float metalness;
    int has_diffuse_map;
    int has_normal_map;
    int has_roughness_map;
    int has_metal_map;
};

uniform sampler2D diffuse_map;
uniform sampler2D metal_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;

// uniform vec3 cam_pos;

void main()
{
    float rness = roughness;
    float mness = metalness;

    if (has_roughness_map == 1)
    {
        rness = texture(roughness_map, tex_coords).x;
    }
    if (has_metal_map == 1)
    {
        mness = texture(metal_map, tex_coords).x;
    }

    rness = clamp(rness, 0.01, 0.99);
    mness = clamp(mness, 0.01, 0.99);

    vec3 N = normal.xyz;
    if (has_normal_map == 1 /*&& distance(world_pos, cam_pos) < 10*/)
    {
        N = texture(normal_map, tex_coords).xyz;

        // Make the normal map work properly
        N = normalize(N * 2.0 - 1.0);
        N = normalize(tbn * N);
    }

    NormalMetallic = vec4(N, mness);

    if (has_diffuse_map == 1)
    {
        vec4 tx = texture(diffuse_map, tex_coords);
        ColorRoughness = vec4(tx.xyz, rness);

        if (tx.w == 0)
        {
            discard;
        }
    }
    else
    {
        ColorRoughness = vec4(color, rness);
    }

    position = world_pos;
}