#version 330 core
out vec4 FragColor;

in vec2 tex_coords;

uniform sampler2D frame;

void main()
{
    vec3 color = texture(frame, tex_coords).xyz;

    // Gamma correction
    FragColor = vec4(pow(color/(color + 1.0), vec3(1.0/2.2)), 1);
    
}