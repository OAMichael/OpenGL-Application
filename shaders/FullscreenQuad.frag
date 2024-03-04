#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uScreenTexture;


void main() {
    const float gamma = 2.2;
    const float exposure = 0.8;
    vec3 hdrColor = texture(uScreenTexture, inUv).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    outColor = vec4(mapped, 1.0);
}
