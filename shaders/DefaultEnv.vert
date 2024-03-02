#version 460 core

#define	BACKGROUND_IMAGE_2D 0
#define	SKYBOX 1
#define	EQUIRECTANGULAR 2

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec3 outUv;


layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

uniform uint uEnvironmentType;


void main()
{
    vec4 pos = vec4(0.0);
    if(uEnvironmentType == BACKGROUND_IMAGE_2D) {
        pos = vec4(inPos, 1.0);
        outUv.xy = inUv;
    }
    else if(uEnvironmentType == SKYBOX) {
        pos = proj * mat4(mat3(view)) * vec4(inPos, 1.0);
        outUv = inPos;
    }
    else if(uEnvironmentType == EQUIRECTANGULAR) {
        pos = proj * mat4(mat3(view)) * vec4(inPos, 1.0);
        outUv = inPos;
    }
    gl_Position = pos.xyww;
}  