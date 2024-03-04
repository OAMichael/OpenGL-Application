#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uScreenTexture;
uniform bool uHorizontal;


float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    ivec2 textureCoords = ivec2(textureSize(uScreenTexture, 0) * inUv);
    vec3 result = texelFetch(uScreenTexture, textureCoords, 0).rgb * weight[0];
    if (uHorizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texelFetch(uScreenTexture, textureCoords + ivec2(i, 0), 0).rgb * weight[i];
            result += texelFetch(uScreenTexture, textureCoords - ivec2(i, 0), 0).rgb * weight[i];
        }
    }
    else {
        for (int i = 1; i < 5; ++i) {
            result += texelFetch(uScreenTexture, textureCoords + ivec2(0, i), 0).rgb * weight[i];
            result += texelFetch(uScreenTexture, textureCoords - ivec2(0, i), 0).rgb * weight[i];
        }
    }
    outColor = vec4(result, 1.0);
}