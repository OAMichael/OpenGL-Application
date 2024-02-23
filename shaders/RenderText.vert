#version 460 core
layout (location = 0) in vec4 inVertex;

layout (location = 0) out vec2 outUv;

uniform mat4 uProj;

void main() {
    gl_Position = uProj * vec4(inVertex.xy, 0.0, 1.0);
    outUv = inVertex.zw;
}
