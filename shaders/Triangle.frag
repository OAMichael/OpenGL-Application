#version 460 core

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;


uniform sampler2D uTexture;


void main()
{
    outColor = vec4(0.2f, 0.3f, 0.3f, 1.0f);

    if(gl_FragCoord.z < 2)
        outColor = texture(uTexture, inUv);
} 