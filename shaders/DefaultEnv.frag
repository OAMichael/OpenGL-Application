#version 460 core

#define	BACKGROUND_IMAGE_2D 0
#define	SKYBOX 1

layout(location = 0) in vec3 inUv;

layout(location = 0) out vec4 outColor;

uniform samplerCube uCubeSampler;
uniform sampler2D uSampler;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

uniform uint environmentType;


void main()
{
    if(environmentType == BACKGROUND_IMAGE_2D)
        outColor = texture(uSampler, inUv.xy);
    else if(environmentType == SKYBOX)
        outColor = texture(uCubeSampler, inUv);
}