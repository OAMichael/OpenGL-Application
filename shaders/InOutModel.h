#ifndef IN_OUT_MODEL_H
#define IN_OUT_MODEL_H

#define TEXTURE_INDEX_BASE_COLOR            0
#define TEXTURE_INDEX_METALLIC_ROUGHNESS    1
#define TEXTURE_INDEX_EMISSIVE              2
#define TEXTURE_INDEX_NORMAL                3
#define TEXTURE_INDEX_OCCLUSION             4

#define TEXTURE_INDEX_COUNT                 5


#define MATERIAL_FLAG_NORMAL_MAP_BIT        1 << 0

#define	BACKGROUND_IMAGE_2D 0
#define	SKYBOX 1
#define	EQUIRECTANGULAR 2

#define PI 3.14159


uniform sampler2D materialTextures[TEXTURE_INDEX_COUNT];
uniform vec4 materialTexturesFactors[TEXTURE_INDEX_COUNT];

uniform samplerCube uCubeSamplerSkybox;
uniform sampler2D uSamplerEquirect;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inUv;

layout(std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

uniform vec3 cameraWorldPos;
uniform uint materialFlags;
uniform uint environmentType;


uniform float time;

out vec4 outColor;


#endif
