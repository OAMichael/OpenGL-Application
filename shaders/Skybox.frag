#version 460 core

layout(location = 0) in vec3 TexCoords;

layout(location = 0) out vec4 outColor;

uniform samplerCube uCubeSampler;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};


void main()
{    
    outColor = texture(uCubeSampler, TexCoords);
}