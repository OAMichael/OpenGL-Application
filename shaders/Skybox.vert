#version 460 core

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 TexCoords;


layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};


void main()
{
    TexCoords = aPos;
    vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  