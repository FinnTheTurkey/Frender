#version 330 core
out vec4 FragColor;

layout (std140) uniform Material 
{
    vec3 color;
};

void main()
{
    FragColor = vec4(color, 1.0f);
} 