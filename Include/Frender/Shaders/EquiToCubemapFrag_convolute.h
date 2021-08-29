// Auto generated file.
static const char EquiToCubemap_convoluteFragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
in vec3 localPos;\n\
\n\
uniform sampler2D equirectangularMap;\n\
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
void main()\n\
{\n\
    vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos\n\
    vec3 color = texture(equirectangularMap, uv).rgb;\n\
\n\
    vec3 normal = normalize(localPos);\n\
    vec3 irradiance = vec3(0.0);\n\
\n\
    vec3 up    = vec3(0.0, 1.0, 0.0);\n\
    vec3 right = normalize(cross(up, normal));\n\
    up         = normalize(cross(normal, right));\n\
\n\
    float sampleDelta = 0.025;\n\
    float nrSamples = 0.0; \n\
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)\n\
    {\n\
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)\n\
        {\n\
            // spherical to cartesian (in tangent space)\n\
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));\n\
            // tangent space to world\n\
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; \n\
\n\
            irradiance += texture(equirectangularMap, SampleSphericalMap(normalize(sampleVec))).rgb * cos(theta) * sin(theta);\n\
            nrSamples++;\n\
        }\n\
    }\n\
    irradiance = PI * irradiance * (1.0 / float(nrSamples));\n\
\n\
    FragColor = vec4(irradiance, 1.0);\n\
}";
