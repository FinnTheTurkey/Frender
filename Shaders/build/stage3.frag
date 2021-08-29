#version 330 core
out vec4 FragColor;

in vec2 tex_coords;

uniform sampler2D frame;
uniform sampler2D bloom_blur;

uniform float bloom_exposure;

void main()
{
    vec3 color = texture(frame, tex_coords).rgb;
    vec3 bloom_color = texture(bloom_blur, tex_coords).rgb;

    color += bloom_color;

    // Tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * 1.0); // 1.0 is exposure
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2)); // 2.2 is gamma

    FragColor = vec4(mapped, 1);
}