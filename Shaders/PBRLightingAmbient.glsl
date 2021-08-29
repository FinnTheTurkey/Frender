const float PI = 3.14159265359;

// Random but important equations
// I'm not even going to pretend to know how PBR lighting works
// Thanks so, so much to Joey DeVries
// https://learnopengl.com/PBR/Lighting
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 computeAmbient(vec3 N, vec3 V, vec3 F0, float roughness, vec3 diffuse_color, vec3 irradiance)
{
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    vec3 diffuse = irradiance * diffuse_color;
    vec3 ambient = (kD * diffuse); // * ao
    return ambient;
}

#pragma glslify: export(computeAmbient)