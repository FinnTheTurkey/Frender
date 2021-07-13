// Auto generated file.
static const char Stage3FragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 tex_coords;\n\
\n\
uniform sampler2D frame;\n\
uniform sampler2D bloom_blur;\n\
\n\
uniform float bloom_exposure;\n\
\n\
void main()\n\
{\n\
    vec3 color = texture(frame, tex_coords).rgb;\n\
    vec3 bloom_color = texture(bloom_blur, tex_coords).rgb;\n\
\n\
    color += bloom_color;\n\
\n\
    // Tone mapping\n\
    vec3 mapped = vec3(1.0) - exp(-color * 1.0); // 1.0 is exposure\n\
    // Gamma correction\n\
    mapped = pow(mapped, vec3(1.0 / 2.2)); // 2.2 is gamma\n\
\n\
    FragColor = vec4(mapped, 1);\n\
    \n\
}";
