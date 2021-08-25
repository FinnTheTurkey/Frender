// Auto generated file.
static const char EquiToCubemapFragSrc[] = "#version 330 core\n\
#define GLSLIFY 1\n\
out vec4 FragColor;\n\
in vec3 localPos;\n\
\n\
uniform sampler2D equirectangularMap;\n\
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
    FragColor = vec4(color, 1.0);\n\
}";
