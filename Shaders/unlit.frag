#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 tex_coords;
in vec3 normal;
in vec3 world_pos;

in mat3 tbn;

layout (std140) uniform Material 
{
    vec3 color;
    float emmisive;
    int has_diffuse_map;
};

uniform sampler2D diffuse_map;

void main()
{
    if (has_diffuse_map == 1)
    {
        vec4 tx = texture(diffuse_map, tex_coords);
        FragColor = vec4(tx.xyz, 1);

        if (tx.w == 0)
        {
            discard;
        }
    }
    else
    {
        FragColor = vec4(color, 1);
    }

    if (emmisive > 1)
    {
        BrightColor = vec4(FragColor.xyz * emmisive, 1);
    }
    else
    {
        BrightColor = vec4(0, 0, 0, 1);
    }
}