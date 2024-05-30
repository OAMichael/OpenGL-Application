#include "../GLSLversion.h"
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

#define FONT_EFFECT_DEFAULT
//#define FONT_EFFECT_OUTLINE_BORDER
//#define FONT_EFFECT_OUTLINE_BORDER_SMOOTH
//#define FONT_EFFECT_SHADOW
//#define FONT_EFFECT_GLOW
//#define FONT_EFFECT_SOFT_GLOW

uniform sampler2D uTextSampler;
uniform vec3 uTextColor;
uniform bool uMSDF;

const float smoothing = 1.0/16.0;
float distCoeff = 5.0f;
float distThreshold = 0.01f;
float shadowThresholdCoeff = 2.0f;

float outlineDistance = 0.03f;
const vec3 outlineColor = vec3(0.1f);
vec2 shadowOffset = vec2(0.02f);
const float shadowSmoothing = 1.0/16.0;
const vec3 shadowColor = vec3(0.0f);
float glowCuttoff = 0.1f;
const float glowFactor = 1.3f;



float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}


void main() {
    float sdf = 0.0f;
    if (uMSDF) {
        vec3 msd = texture(uTextSampler, inUv).rgb;
        sdf = 2.0f * median(msd.r, msd.g, msd.b) - 1.0f;

        outlineDistance *= 24.0f;
        distCoeff *= 0.05f;
        distThreshold *= 20.0f;
        shadowOffset *= 2.0f;
        shadowThresholdCoeff *= 10.0f;
        glowCuttoff *= 8.0f;
    }
    else {
        sdf = texture(uTextSampler, inUv).r * 2.0f - 1.0f;
    }


    if (sdf > 0.0f) {
        outColor = vec4(uTextColor, 1.0f);
    }
#ifdef FONT_EFFECT_DEFAULT
    else if (sdf > -distThreshold) {
        outColor = vec4(uTextColor, smoothstep(0.5f - smoothing, 0.5f + smoothing, 0.5f + smoothing + distCoeff * sdf));
    }
#endif
#ifdef FONT_EFFECT_OUTLINE_BORDER
    else if (sdf > -outlineDistance) {
        outColor = vec4(outlineColor, 1.0f);
    }
#endif
#ifdef FONT_EFFECT_OUTLINE_BORDER_SMOOTH
    else if (sdf > -outlineDistance) {
        float alpha = smoothstep(0.0f, 1.0f, 1.0f + 2.0f * sdf / outlineDistance);
        vec3 color = mix(outlineColor, uTextColor, alpha);
        outColor = vec4(color, smoothstep(0.5f, 1.0f, 240.0f * (outlineDistance + sdf)));
    }
#endif
#ifdef FONT_EFFECT_SHADOW
    else if (sdf > -shadowThresholdCoeff * length(shadowOffset)) {
        float shadowDistance = 0.0f;
        if (uMSDF) {
            vec3 msd = texture(uTextSampler, inUv - shadowOffset).rgb;
            shadowDistance = 2.0f * median(msd.r, msd.g, msd.b) - 1.0f;
        }
        else {
            shadowDistance = texture2D(uTextSampler, inUv - shadowOffset).r * 2.0f - 1.0f;
        }
        float shadowAlpha = shadowDistance > 0.0f ? 1.0f : 0.0f;
        outColor = vec4(shadowColor, shadowAlpha);
    }
#endif
#ifdef FONT_EFFECT_GLOW
    else if (sdf > -glowCuttoff) {
        outColor = vec4(glowFactor * uTextColor, smoothstep(0.5f - glowCuttoff, 0.5f, 0.5f + sdf));
    }
#endif
#ifdef FONT_EFFECT_SOFT_GLOW
    else if (sdf > -glowCuttoff) {
        float glow = mix(1.0f, glowFactor, sqrt(-sdf / glowCuttoff));
        outColor = vec4(glow * uTextColor, smoothstep(0.5f - glowCuttoff, 0.5f, 0.5f + sdf));
    }
#endif
    else {
        outColor = vec4(0.0f);
    }
}
