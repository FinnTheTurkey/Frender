#version 330 core
#define GLSLIFY 1
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tang;
layout (location = 4) in vec3 bitang;

layout (location = 5) in vec3 light_color_arg;
layout (location = 6) in vec3 light_pos_arg;
layout (location = 7)  in float radius_arg;
layout (location = 8) in mat4 mvp;

out vec2 tex_coords;

out vec3 light_color;
out vec3 light_pos;
out float radius;

// uniform mat4 mvp;
// uniform mat4 model;

void main()
{
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coords = tex_coord;

    light_color = light_color_arg;
    light_pos = light_pos_arg;
    radius = radius_arg;
}