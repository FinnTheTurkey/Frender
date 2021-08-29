// Auto generated file.
static const char BulkStage1VertSrc[] = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 normals;\n\
layout (location = 2) in vec2 tex_coord;\n\
layout (location = 3) in vec3 tang;\n\
layout (location = 4) in vec3 bitang;\n\
layout (location = 5) in mat4 mvp;\n\
layout (location = 9) in mat4 model;\n\
\n\
out vec2 tex_coords;\n\
out vec3 normal;\n\
out vec3 world_pos;\n\
\n\
out mat3 tbn;\n\
\n\
// uniform mat4 mvp;\n\
// uniform mat4 model;\n\
\n\
void main()\n\
{\n\
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
    tex_coords = tex_coord;\n\
    normal = vec3(normalize(model * vec4(normals, 0.0)));\n\
    world_pos = vec3(model * vec4(aPos, 1.0f));\n\
\n\
    // TBN matrix\n\
    // We could put this in an if, but that would make no difference\n\
    vec3 T = normalize(vec3(model * vec4(tang, 0.0)));\n\
    vec3 B = normalize(vec3(model * vec4(bitang, 0.0)));\n\
    vec3 N = vec3(normal);\n\
    tbn = mat3(T, B, N);\n\
}";
