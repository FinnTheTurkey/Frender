#version 330 core
out vec4 FragColor;

in vec2 tex_coords;

uniform sampler2D frame;

void main()
{
    vec3 color = texture(frame, tex_coords).xyz;

    // Tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * 1.0); // 1.0 is exposure
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2)); // 2.2 is gamma

    FragColor = vec4(mapped, 1);
    
}