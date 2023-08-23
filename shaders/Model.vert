#version 460 core

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec2 outTexCoord;


void main() {
	mat4 MVP = proj * view * model;
	
	gl_Position = MVP * vec4(inVertex, 1);
	outNormal = normalize(transpose(inverse(mat3(model))) * inNormal);
	outPosition = (model * vec4(inVertex, 1.0f)).xyz;
	outTexCoord = inTexCoord;
}
