#include "../GLSLversion.h"
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uTextSampler;
uniform vec3 uTextColor;

void main() {
    float sdf = texture(uTextSampler, inUv).r;
    sdf = sdf * 2.0f - 1.0f;

    if (sdf > 0.0f) {
        outColor = vec4(uTextColor, 1.0f);
    }
    else if (sdf > -0.01f) {
        outColor = vec4(uTextColor, smoothstep(0.0f, 1.0f, 1.0f + 100.0f * sdf));
    }
    else {
        outColor = vec4(0.0f);
    }
}  