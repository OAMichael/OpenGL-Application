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

const float smoothing = 1.0/16.0;

const float outlineDistance = 0.08f;
const vec3 outlineColor = vec3(0.1f);
const vec2 shadowOffset = vec2(0.02f);
const float shadowSmoothing = 1.0/16.0;
const vec3 shadowColor = vec3(0.0f);
const float glowCuttoff = 0.1f;
const float glowFactor = 1.3f;

void main() {
    float sdfSample = texture(uTextSampler, inUv).r;
    float sdf = sdfSample * 2.0f - 1.0f;

    if (sdf > 0.0f) {
        outColor = vec4(uTextColor, 1.0f);
    }
#ifdef FONT_EFFECT_DEFAULT
    else if (sdf > -0.01f) {
        outColor = vec4(uTextColor, smoothstep(0.5f - smoothing, 0.5f + smoothing, 1.0f - sdfSample));
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
        outColor = vec4(color, smoothstep(0.5f, 1.0f, 40.0f * (outlineDistance + sdf)));
    }
#endif
#ifdef FONT_EFFECT_SHADOW
    else if (sdf > -2.0 * length(shadowOffset)) {
        float shadowDistance = texture2D(uTextSampler, inUv - shadowOffset).r * 2.0f - 1.0f;
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