#version 330 core
out vec4 FragColor;

in vec2 tex_coords;

layout (std140) uniform Material 
{
    vec3 color;
    int has_texture;
};

uniform sampler2D tex;

void main()
{
    // FragColor = vec4(color, 1.0f);
    if (has_texture == 1)
    {
        FragColor = texture(tex, tex_coords);
    }
    else
    {
        FragColor = vec4(color, 1.0f);
    }
} 