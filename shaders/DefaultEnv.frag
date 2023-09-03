#version 460 core

#define	BACKGROUND_IMAGE_2D 0
#define	SKYBOX 1
#define	EQUIRECTANGULAR 2

#define PI 3.14159

layout(location = 0) in vec3 inUv;

layout(location = 0) out vec4 outColor;

uniform sampler2D uSamplerBackground2D;
uniform samplerCube uSamplerSkybox;
uniform sampler2D uSamplerEquirect;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

uniform uint environmentType;


void main()
{
    if(environmentType == BACKGROUND_IMAGE_2D)
        outColor = texture(uSamplerBackground2D, inUv.xy);
    else if(environmentType == SKYBOX)
        outColor = texture(uSamplerSkybox, inUv);
    else if(environmentType == EQUIRECTANGULAR) {
        const vec3 normUv = normalize(inUv);
        const float phi = atan(normUv.z, normUv.x);
        const float psi = asin(-normUv.y);
        const float u = phi / 2.0 / PI;
        const float v = 0.5f + 1.0 / PI * psi;

        outColor = texture(uSamplerEquirect, vec2(u, v));
    }
}