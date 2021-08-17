#version 330 core
#define GLSLIFY 1
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tang;
layout (location = 4) in vec3 bitang;
out vec2 tex_coords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coords = tex_coord;
}