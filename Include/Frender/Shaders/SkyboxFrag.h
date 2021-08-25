// Auto generated file.
static const char SkyboxFragSrc[] = "#version 330 core\n\
#define GLSLIFY 1\n\
layout (location = 0) out vec4 FragColor;\n\
layout (location = 1) out vec4 BrightColor;\n\
\n\
in vec3 tex_coords;\n\
\n\
uniform samplerCube skybox;\n\
\n\
void main()\n\
{\n\
    // FragColor = texture(skybox, tex_coords).rbga;\n\
    // FragColor \n\
    vec3 envColor = texture(skybox, tex_coords).rgb;\n\
    \n\
    envColor = envColor / (envColor + vec3(1.0));\n\
    // envColor = pow(envColor, vec3(1.0/2.2)); \n\
  \n\
    FragColor = vec4(envColor, 1.0);\n\
    // If it's higher than a certain point, it goes into\n\
    // The bloom buffer\n\
    // float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));\n\
    // if(brightness > 1.0)\n\
    //     BrightColor = vec4(FragColor.rgb, 1.0);\n\
    // else\n\
    //     BrightColor = vec4(0.0, 0.0, 0.0, 1.0);\n\
}";
