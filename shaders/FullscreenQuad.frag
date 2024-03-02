#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uScreenTexture;


void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(uScreenTexture, inUv).rgb;

    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));

    outColor = vec4(mapped, 1.0);
}
