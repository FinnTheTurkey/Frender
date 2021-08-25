#version 330 core
#define GLSLIFY 1
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 tex_coords;

uniform samplerCube skybox;

void main()
{
    // FragColor = texture(skybox, tex_coords).rbga;
    // FragColor 
    vec3 envColor = texture(skybox, tex_coords).rgb;
    
    envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0/2.2)); 
  
    FragColor = vec4(envColor, 1.0);
    // If it's higher than a certain point, it goes into
    // The bloom buffer
    // float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    // if(brightness > 1.0)
    //     BrightColor = vec4(FragColor.rgb, 1.0);
    // else
    //     BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}