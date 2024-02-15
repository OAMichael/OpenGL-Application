#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uScreenTexture;
uniform bool uEnableBlur;

ivec2 offsets[9] = ivec2[](
    ivec2(-1,  1),
    ivec2( 0,  1),
    ivec2( 1,  1),
    ivec2(-1,  0),
    ivec2( 0,  0),
    ivec2( 1,  0),
    ivec2(-1, -1),
    ivec2( 0, -1),
    ivec2( 1, -1)   
);

float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
);


void main() {
    vec3 color = vec3(0.0f);
    if (uEnableBlur) {
        ivec2 textureCoords = ivec2(textureSize(uScreenTexture, 0) * inUv);

        for (int i = 0; i < 9; i++) {
            color += texelFetch(uScreenTexture, textureCoords + offsets[i], 0).rgb * kernel[i];
        }
    }
    else {
        color = texture(uScreenTexture, inUv).rgb;
    }
    gl_FragColor = vec4(color, 1.0);
}
