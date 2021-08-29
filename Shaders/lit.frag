#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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

layout (std140) uniform Lights
{
    vec4 light_pos_dir_rad[256];
    vec4 light_color_type[256]; // 0 for point light, 1 for directional light
};

uniform sampler2D diffuse_map;
uniform sampler2D metal_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;

uniform samplerCube irradiance_map;

// uniform int width;
// uniform int height;
uniform vec3 cam_pos;

in vec4 lights_part1;
in vec4 lights_part2;

#include "PBRLighting.glsl"

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

    vec3 N = normal.xyz;
    if (has_normal_map == 1)
    {
        N = texture(normal_map, tex_coords).xyz;

        // Make the normal map work properly
        N = normalize(N * 2.0 - 1.0);
        N = normalize(tbn * N);
    }

    // NormalMetallic = vec4(N, mness);
    vec3 colour = color;

    if (has_diffuse_map == 1)
    {
        vec4 tx = texture(diffuse_map, tex_coords);
        colour = tx.xyz;

        if (tx.w == 0)
        {
            discard;
        }
    }

    vec3 V = normalize(cam_pos - world_pos.xyz);
    vec3 F0 = vec3(0.4);
    F0 = mix(F0, colour.xyz, mness);

    vec3 end_result;

    for (int i = 0; i < 4; i++)
    {
        if (lights_part1[i] != -1.0)
        {
            end_result += reflectanceEquation(light_color_type[int(lights_part1[i])].w, N, V, F0, colour.xyz,
                    light_pos_dir_rad[int(lights_part1[i])].xyz,
                    world_pos, light_color_type[int(lights_part1[i])].xyz, rness, mness, light_pos_dir_rad[int(lights_part1[i])].w);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (lights_part2[i] != -1.0)
        {
            end_result += reflectanceEquation(light_color_type[int(lights_part2[i])].w, N, V, F0, colour.xyz,
                    light_pos_dir_rad[int(lights_part2[i])].xyz,
                    world_pos, light_color_type[int(lights_part2[i])].xyz, rness, mness, light_pos_dir_rad[int(lights_part2[i])].w);
        }
    }

    end_result += computeAmbient(N, V, F0, rness, colour.xyz, texture(irradiance_map, N).xyz);

    FragColor = vec4(end_result, 1);
    // FragColor = vec4(0.01, 0.01, 0.01, 1);

    // If it's higher than a certain point, it goes into
    // The bloom buffer
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}