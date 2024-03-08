#version 460 core

#include "Constants.h"

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

uniform uint uEnvironmentType;
uniform bool uIsHdr;


void main()
{
    if(uEnvironmentType == BACKGROUND_IMAGE_2D)
        outColor = texture(uSamplerBackground2D, inUv.xy);
    else if(uEnvironmentType == SKYBOX)
        outColor = texture(uSamplerSkybox, inUv);
    else if(uEnvironmentType == EQUIRECTANGULAR) {
        outColor = texture(uSamplerEquirect, CubemapToEquirect(normalize(inUv)));
    }

    if (!uIsHdr) {
        // Yes, it is expensive but I don't care
        const float exposure = 0.8f;
        outColor.rgb = -1.0 / exposure * log(1 -  pow(outColor.rgb, vec3(2.2f)));
    }
}