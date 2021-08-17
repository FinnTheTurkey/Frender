#version 330 core
#define GLSLIFY 1
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tang;
layout (location = 4) in vec3 bitang;
layout (location = 5) in mat4 mvp;
layout (location = 9) in mat4 model;

out vec2 tex_coords;
out vec3 normal;
out vec3 world_pos;

out mat3 tbn;

// uniform mat4 mvp;
// uniform mat4 model;

void main()
{
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    tex_coords = tex_coord;
    normal = vec3(normalize(model * vec4(normals, 0.0)));
    world_pos = vec3(model * vec4(aPos, 1.0f));

    // TBN matrix
    // We could put this in an if, but that would make no difference
    vec3 T = normalize(vec3(model * vec4(tang, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitang, 0.0)));
    vec3 N = vec3(normal);
    tbn = mat3(T, B, N);
}