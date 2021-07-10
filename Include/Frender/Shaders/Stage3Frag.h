// Auto generated file.
static const char Stage3FragSrc[] = "#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 tex_coords;\n\
\n\
uniform sampler2D frame;\n\
\n\
void main()\n\
{\n\
    vec3 color = texture(frame, tex_coords).xyz;\n\
\n\
    // Gamma correction\n\
    FragColor = vec4(pow(color/(color + 1.0), vec3(1.0/2.2)), 1);\n\
    \n\
}";
