// Auto generated file.
static const char IntegrateBRDFFragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
in vec2 tex_coords;\n\
\n\
const float PI = 3.14159265359;\n\
\n\
const vec2 invAtan = vec2(0.1591, 0.3183);\n\
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
vec2 IntegrateBRDF(float NdotV, float roughness)\n\
{\n\
    vec3 V;\n\
    V.x = sqrt(1.0 - NdotV*NdotV);\n\
    V.y = 0.0;\n\
    V.z = NdotV;\n\
\n\
    float A = 0.0;\n\
    float B = 0.0;\n\
\n\
    vec3 N = vec3(0.0, 0.0, 1.0);\n\
\n\
    const uint SAMPLE_COUNT = 1024u;\n\
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)\n\
    {\n\
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);\n\
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);\n\
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);\n\
\n\
        float NdotL = max(L.z, 0.0);\n\
        float NdotH = max(H.z, 0.0);\n\
        float VdotH = max(dot(V, H), 0.0);\n\
\n\
        if(NdotL > 0.0)\n\
        {\n\
            float G = geometrySmith(N, V, L, roughness);\n\
            float G_Vis = (G * VdotH) / (NdotH * NdotV);\n\
            float Fc = pow(1.0 - VdotH, 5.0);\n\
\n\
            A += (1.0 - Fc) * G_Vis;\n\
            B += Fc * G_Vis;\n\
        }\n\
    }\n\
    A /= float(SAMPLE_COUNT);\n\
    B /= float(SAMPLE_COUNT);\n\
    return vec2(A, B);\n\
}\n\
\n\
\n\
void main() \n\
{\n\
    vec2 integratedBRDF = IntegrateBRDF(tex_coords.x, tex_coords.y);\n\
    FragColor = vec4(integratedBRDF, 0, 0);\n\
}";
