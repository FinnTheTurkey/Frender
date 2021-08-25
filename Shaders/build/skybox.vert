#version 330 core
#define GLSLIFY 1
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tx_coords;

out vec3 tex_coords;

uniform mat4 vp;

void main()
{
    tex_coords = aPos;
    vec4 pos = vp * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}