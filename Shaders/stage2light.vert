#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tang;
layout (location = 4) in vec3 bitang;
out vec2 tex_coords;

uniform mat4 mvp;
uniform mat4 model;

void main()
{
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coords = tex_coord;
}