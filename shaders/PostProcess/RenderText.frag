#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uTextSampler;
uniform vec3 uTextColor;

void main() {
    outColor = vec4(uTextColor, texture(uTextSampler, inUv).r);
}  