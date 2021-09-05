// Auto generated file.
static const char EquiToCubemap_prefilterFragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
in vec3 localPos;\n\
\n\
uniform sampler2D equirectangularMap;\n\
uniform float roughness;\n\
\n\
const float PI = 3.14159265359;\n\
\n\
const vec2 invAtan = vec2(0.1591, 0.3183);\n\
vec2 SampleSphericalMap(vec3 v)\n\
{\n\
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));\n\
    uv *= invAtan;\n\
    uv += 0.5;\n\
    return uv;\n\
}\n\
\n\
float RadicalInverse_VdC(uint bits) \n\
{\n\
    bits = (bits << 16u) | (bits >> 16u);\n\
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n\
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n\
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n\
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n\
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000\n\
}\n\
\n\
vec2 Hammersley(uint i, uint N)\n\
{\n\
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));\n\
}\n\
\n\
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)\n\
{\n\
    float a = roughness*roughness;\n\
\n\
    float phi = 2.0 * PI * Xi.x;\n\
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));\n\
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);\n\
\n\
    // from spherical coordinates to cartesian coordinates\n\
    vec3 H;\n\
    H.x = cos(phi) * sinTheta;\n\
    H.y = sin(phi) * sinTheta;\n\
    H.z = cosTheta;\n\
\n\
    // from tangent-space vector to world-space sample vector\n\
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n\
    vec3 tangent   = normalize(cross(up, N));\n\
    vec3 bitangent = cross(N, tangent);\n\
\n\
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;\n\
    return normalize(sampleVec);\n\
}\n\
\n\
void main()\n\
{\n\
    vec3 N = normalize(localPos);\n\
    vec3 R = N;\n\
    vec3 V = R;\n\
\n\
    const uint SAMPLE_COUNT = 1024u;\n\
    float totalWeight = 0.0;   \n\
    vec3 prefilteredColor = vec3(0.0);\n\
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)\n\
    {\n\
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);\n\
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);\n\
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);\n\
\n\
        float NdotL = max(dot(N, L), 0.0);\n\
        if(NdotL > 0.0)\n\
        {\n\
            prefilteredColor += texture(equirectangularMap, SampleSphericalMap(normalize(L))).rgb * NdotL;\n\
            totalWeight      += NdotL;\n\
        }\n\
    }\n\
    prefilteredColor = prefilteredColor / totalWeight;\n\
            // irradiance += texture(equirectangularMap, SampleSphericalMap(normalize(sampleVec))).rgb * cos(theta) * sin(theta);\n\
    FragColor = vec4(prefilteredColor, 1.0);\n\
}";
