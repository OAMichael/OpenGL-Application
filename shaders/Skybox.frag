#version 460 core

layout(location = 0) in vec3 TexCoords;

layout(location = 0) out vec4 outColor;


uniform samplerCube uCubeSampler;


void main()
{    
    outColor = texture(uCubeSampler, TexCoords);
}