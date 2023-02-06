#version 460 core
layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 TexCoords;
uniform mat4 transform;

void main()
{
    TexCoords = aPos;
    gl_Position = transform * vec4(aPos, 1.0);
}  