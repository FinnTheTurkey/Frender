#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 tex_coords;

uniform sampler2D ColorRoughness;
uniform sampler2D NormalMetal;
uniform sampler2D position;

uniform int width;
uniform int height;
uniform vec3 cam_pos;

// Old method
// uniform vec3 light_color;
// uniform vec3 light_pos;
// uniform float radius;

// New method
in vec3 light_color;
in vec3 light_pos;
in float radius;


#pragma glslify: reflectanceEquation = require(./PBRLighting.glsl)

void main()
{
    vec2 screen_pos = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);
    // FragColor = vec4(texture(ColorRoughness, vec2(gl_FragCoord.x/width, gl_FragCoord.y/height)).xyz, 1.0f);
    vec4 color_roughness = texture(ColorRoughness, screen_pos);
    vec3 color = color_roughness.xyz;
    float roughness = color_roughness.w;

    vec4 normal_metal = texture(NormalMetal, screen_pos);
    vec3 normal = normal_metal.xyz;
    float metal = normal_metal.w;

    vec3 pos = texture(position, screen_pos).xyz;

    vec3 V = normalize(cam_pos - pos.xyz);
    vec3 F0 = vec3(0.4);
    F0 = mix(F0, color.xyz, metal);

    vec3 end_result = reflectanceEquation(0.0, normal, V, F0, color.xyz, light_pos, pos,
            light_color, roughness, metal, radius);

    FragColor = vec4(end_result, 1);
    // FragColor = vec4(0.01, 0.01, 0.01, 1);

    // If it's higher than a certain point, it goes into
    // The bloom buffer
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}