// Auto generated file.
static const char LitFragSrc[] = "#version 330 core\n\
layout (location = 0) out vec4 FragColor;\n\
layout (location = 1) out vec4 BrightColor;\n\
\n\
in vec2 tex_coords;\n\
in vec3 normal;\n\
in vec3 world_pos;\n\
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
layout (std140) uniform Lights\n\
{\n\
    vec4 light_pos_dir_rad[256];\n\
    vec4 light_color_type[256]; // 0 for point light, 1 for directional light\n\
};\n\
\n\
uniform sampler2D diffuse_map;\n\
uniform sampler2D metal_map;\n\
uniform sampler2D normal_map;\n\
uniform sampler2D roughness_map;\n\
\n\
uniform samplerCube irradiance_map;\n\
uniform samplerCube prefilter_map;\n\
uniform sampler2D brdf;\n\
\n\
// uniform int width;\n\
// uniform int height;\n\
uniform vec3 cam_pos;\n\
\n\
in vec4 lights_part1;\n\
in vec4 lights_part2;\n\
\n\
\n\
// Included file: PBRLighting.glsl\n\
const float PI = 3.14159265359;\n\
\n\
// Random but important equations\n\
// I'm not even going to pretend to know how PBR lighting works\n\
// Thanks so, so much to Joey DeVries\n\
// https://learnopengl.com/PBR/Lighting\n\
vec3 fresnelSchlick(float cos_theta, vec3 F0)\n\
{\n\
    return F0 + (1.0 - F0) * pow(max(1.0 - cos_theta, 0.0), 5.0);\n\
}\n\
\n\
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)\n\
{\n\
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);\n\
}\n\
\n\
float distributionGGX(vec3 N, vec3 H, float roughness_p)\n\
{\n\
    float a = roughness_p * roughness_p;\n\
    float a2 = a * a;\n\
    float NdotH = max(dot(N, H), 0.0);\n\
    float NdotH2 = NdotH * NdotH;\n\
\n\
    float num = a2;\n\
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n\
    denom = PI * denom * denom;\n\
\n\
    return num / denom;\n\
}\n\
\n\
float geometrySchlickGGX(float NdotV, float roughness_p)\n\
{\n\
    float r = (roughness_p + 1.0);\n\
    float k = (r*r) / 8.0;\n\
\n\
    float num = NdotV;\n\
    float denom = NdotV * (1.0 - k) + k;\n\
\n\
    return num / denom;\n\
}\n\
\n\
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness_p)\n\
{\n\
    float NdotV = max(dot(N, V), 0.0);\n\
    float NdotL = max(dot(N, L), 0.0);\n\
    float ggx2 = geometrySchlickGGX(NdotV, roughness_p);\n\
    float ggx1 = geometrySchlickGGX(NdotL, roughness_p);\n\
\n\
    return ggx1 * ggx2;\n\
}\n\
\n\
/*\n\
Light Infos: Vec3(enum light type, float radius, float light_infos.z)\n\
Types:\n\
0 - Point light\n\
1 - Directional light\n\
2 - Spot light\n\
*/\n\
vec3 reflectanceEquation(float light_type, vec3 N, vec3 V, vec3 F0, vec3 diffuse, vec3 light_pos, vec3 world_pos,\n\
                        vec3 light_color, float roughness_p, float metal_p, float radius)\n\
{\n\
    // Calculate per light radiance\n\
    vec3 L;\n\
    if (light_type == 1.0) // Directional light\n\
    {\n\
        L = normalize(-light_pos);\n\
    }\n\
    else\n\
    {\n\
        L = normalize(light_pos - world_pos.xyz);\n\
    }\n\
\n\
    vec3 H = normalize(V + L);\n\
    // float attenuation = 1.0 / (distance * distance);\n\
    vec3 radiance;\n\
    if (light_type == 0.0)\n\
    {\n\
        float distance = length(light_pos - world_pos.xyz);\n\
        // float attenuation = pow(1.0-pow((distance/(radius/2)),4.0),2.0)/distance*distance+1.0;\n\
        // float attenuation = 1.0 / (distance * distance);\n\
        float attenuation = clamp(1.0 - distance/(radius/1.5), 0.0, 1.0);\n\
        radiance = light_color * attenuation;\n\
    }\n\
    // else if (light_infos.x == 2.0) // Spot light\n\
    // {\n\
    //     float theta = dot(L, normalize(-light_direction));\n\
    //     float inner_cutoff = (light_infos.z * 1.1);\n\
    //     float epsilon = inner_cutoff - light_infos.z;\n\
    //     float attenuation = clamp((theta - light_infos.z) / epsilon, 0.0, 1.0);\n\
    //     radiance = light_color * attenuation;\n\
    // }\n\
    else // Directional light\n\
    {\n\
        radiance = light_color;\n\
    }\n\
\n\
    // Cook-Torrance BRDF\n\
    float NDF = distributionGGX(N, H, roughness_p);\n\
    float G = geometrySmith(N, V, L, roughness_p);\n\
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);\n\
\n\
    vec3 kS = F;\n\
    vec3 kD = vec3(1.0) - kS;\n\
    kD *= 1.0 - metal_p;\n\
\n\
    vec3 numerator = NDF * G * F;\n\
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);\n\
    vec3 specular = numerator / max(denominator, 0.001);\n\
\n\
    // Add to outgoing radience Lo\n\
    float NdotL = max(dot(N, L), 0.0);\n\
    return (kD * diffuse / PI + specular) * radiance * NdotL;\n\
}\n\
\n\
vec3 computeAmbient(vec3 N, vec3 V, vec3 F0, float roughness, vec3 diffuse_color, vec3 irradiance,\n\
                samplerCube prefiltered, sampler2D brdf)\n\
{\n\
    vec3 R = reflect(-V, N);\n\
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);\n\
    vec3 F = kS;\n\
    vec3 kD = 1.0 - kS;\n\
    vec3 diffuse = irradiance * diffuse_color;\n\
\n\
    const float MAX_REFLECTION_LOD = 4.0;\n\
    vec3 prefilteredColor = textureLod(prefiltered, R,  roughness * MAX_REFLECTION_LOD).rgb;   \n\
    vec2 envBRDF  = texture(brdf, vec2(max(dot(N, V), 0.0), roughness)).rg;\n\
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);\n\
\n\
    vec3 ambient = (kD * diffuse + specular); // * ao\n\
    return ambient;\n\
}\n\
\n\
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
    rness = clamp(rness, 0.01, 0.99);\n\
    mness = clamp(mness, 0.01, 0.99);\n\
\n\
    vec3 N = normal.xyz;\n\
    if (has_normal_map == 1)\n\
    {\n\
        N = texture(normal_map, tex_coords).xyz;\n\
\n\
        // Make the normal map work properly\n\
        N = normalize(N * 2.0 - 1.0);\n\
        N = normalize(tbn * N);\n\
    }\n\
\n\
    // NormalMetallic = vec4(N, mness);\n\
    vec3 colour = color;\n\
\n\
    if (has_diffuse_map == 1)\n\
    {\n\
        vec4 tx = texture(diffuse_map, tex_coords);\n\
        colour = tx.xyz;\n\
\n\
        if (tx.w == 0)\n\
        {\n\
            discard;\n\
        }\n\
    }\n\
\n\
    vec3 V = normalize(cam_pos - world_pos.xyz);\n\
    vec3 F0 = vec3(0.4);\n\
    F0 = mix(F0, colour.xyz, mness);\n\
\n\
    vec3 end_result;\n\
\n\
    for (int i = 0; i < 4; i++)\n\
    {\n\
        if (lights_part1[i] != -1.0)\n\
        {\n\
            end_result += reflectanceEquation(light_color_type[int(lights_part1[i])].w, N, V, F0, colour.xyz,\n\
                    light_pos_dir_rad[int(lights_part1[i])].xyz,\n\
                    world_pos, light_color_type[int(lights_part1[i])].xyz, rness, mness, light_pos_dir_rad[int(lights_part1[i])].w);\n\
        }\n\
    }\n\
\n\
    for (int i = 0; i < 4; i++)\n\
    {\n\
        if (lights_part2[i] != -1.0)\n\
        {\n\
            end_result += reflectanceEquation(light_color_type[int(lights_part2[i])].w, N, V, F0, colour.xyz,\n\
                    light_pos_dir_rad[int(lights_part2[i])].xyz,\n\
                    world_pos, light_color_type[int(lights_part2[i])].xyz, rness, mness, light_pos_dir_rad[int(lights_part2[i])].w);\n\
        }\n\
    }\n\
\n\
    end_result += computeAmbient(N, V, F0, rness, colour.xyz, texture(irradiance_map, N).xyz,\n\
                prefilter_map, brdf);\n\
\n\
    FragColor = vec4(end_result, 1);\n\
    // FragColor = vec4(0.01, 0.01, 0.01, 1);\n\
\n\
    // If it's higher than a certain point, it goes into\n\
    // The bloom buffer\n\
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));\n\
    if(brightness > 1.0)\n\
        BrightColor = vec4(FragColor.rgb, 1.0);\n\
    else\n\
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);\n\
}";
