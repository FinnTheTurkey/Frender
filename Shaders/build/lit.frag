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
uniform samplerCube prefilter_map;
uniform sampler2D brdf;

// uniform int width;
// uniform int height;
uniform vec3 cam_pos;

in vec4 lights_part1;
in vec4 lights_part2;


// Included file: PBRLighting.glsl
const float PI = 3.14159265359;

// Random but important equations
// I'm not even going to pretend to know how PBR lighting works
// Thanks so, so much to Joey DeVries
// https://learnopengl.com/PBR/Lighting
vec3 fresnelSchlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cos_theta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness_p)
{
    float a = roughness_p * roughness_p;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness_p)
{
    float r = (roughness_p + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness_p)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness_p);
    float ggx1 = geometrySchlickGGX(NdotL, roughness_p);

    return ggx1 * ggx2;
}

/*
Light Infos: Vec3(enum light type, float radius, float light_infos.z)
Types:
0 - Point light
1 - Directional light
2 - Spot light
*/
vec3 reflectanceEquation(float light_type, vec3 N, vec3 V, vec3 F0, vec3 diffuse, vec3 light_pos, vec3 world_pos,
                        vec3 light_color, float roughness_p, float metal_p, float radius)
{
    // Calculate per light radiance
    vec3 L;
    if (light_type == 1.0) // Directional light
    {
        L = normalize(-light_pos);
    }
    else
    {
        L = normalize(light_pos - world_pos.xyz);
    }

    vec3 H = normalize(V + L);
    // float attenuation = 1.0 / (distance * distance);
    vec3 radiance;
    if (light_type == 0.0)
    {
        float distance = length(light_pos - world_pos.xyz);
        // float attenuation = pow(1.0-pow((distance/(radius/2)),4.0),2.0)/distance*distance+1.0;
        // float attenuation = 1.0 / (distance * distance);
        float attenuation = clamp(1.0 - distance/(radius/1.5), 0.0, 1.0);
        radiance = light_color * attenuation;
    }
    // else if (light_infos.x == 2.0) // Spot light
    // {
    //     float theta = dot(L, normalize(-light_direction));
    //     float inner_cutoff = (light_infos.z * 1.1);
    //     float epsilon = inner_cutoff - light_infos.z;
    //     float attenuation = clamp((theta - light_infos.z) / epsilon, 0.0, 1.0);
    //     radiance = light_color * attenuation;
    // }
    else // Directional light
    {
        radiance = light_color;
    }

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness_p);
    float G = geometrySmith(N, V, L, roughness_p);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal_p;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    // Add to outgoing radience Lo
    float NdotL = max(dot(N, L), 0.0);
    return (kD * diffuse / PI + specular) * radiance * NdotL;
}

vec3 computeAmbient(vec3 N, vec3 V, vec3 F0, float roughness, vec3 diffuse_color, vec3 irradiance,
                samplerCube prefiltered, sampler2D brdf)
{
    vec3 R = reflect(-V, N);
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 F = kS;
    vec3 kD = 1.0 - kS;
    vec3 diffuse = irradiance * diffuse_color;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefiltered, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular); // * ao
    return ambient;
}


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

    end_result += computeAmbient(N, V, F0, rness, colour.xyz, texture(irradiance_map, N).xyz,
                prefilter_map, brdf);

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