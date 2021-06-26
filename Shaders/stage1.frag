#version 330 core
out vec4 FragColor;

in vec2 tex_coords;

layout (std140) uniform Material 
{
    vec3 color;
};

uniform sampler2D tex;

void main()
{
    // FragColor = vec4(color, 1.0f);
    FragColor = texture(tex, tex_coords);
} 