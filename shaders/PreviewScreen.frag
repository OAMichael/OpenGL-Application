#include "GLSLversion.h"

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uScreenTexture;
uniform float uAlpha;


void main() {
    outColor = vec4(texture(uScreenTexture, inUv).rgb, uAlpha);
}
