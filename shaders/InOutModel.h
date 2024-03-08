#ifndef IN_OUT_MODEL_H
#define IN_OUT_MODEL_H

#include "Constants.h"

uniform sampler2D uMaterialTextures[TEXTURE_INDEX_COUNT];
uniform vec4 uMaterialTexturesFactors[TEXTURE_INDEX_COUNT];

uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLUT;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inUv;

layout(std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};


layout(std430, binding = 1) buffer SSBO_Lights 
{
	LightData lights;
};


uniform vec3 uCameraWorldPos;
uniform uint uMaterialFlags;
uniform uint uEnvironmentType;

out vec4 outColor;


#endif
