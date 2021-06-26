#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 tex_coord;
out vec2 tex_coords;

uniform mat4 mvp;
uniform mat4 model;

void main()
{
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coords = tex_coord;
}