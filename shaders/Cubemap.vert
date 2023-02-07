#version 460 core

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 TexCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main()
{
    TexCoords = aPos;
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}  