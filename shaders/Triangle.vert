#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 outUv;


uniform mat4 transform;


void main()
{
    gl_Position = transform * vec4(inPos, 1.0);
    outUv = inUv;
}