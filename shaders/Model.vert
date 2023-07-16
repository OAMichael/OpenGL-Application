#version 460 core

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


out vec3 normal;
out vec3 position;
out vec2 texcoord;


void main() {
	mat4 MVP = proj * view * model;
	
	gl_Position = MVP * vec4(in_vertex, 1);
	normal = normalize(transpose(inverse(mat3(model))) * in_normal);
	position = (model * vec4(in_vertex, 1.0f)).xyz;
	texcoord = in_texcoord;
}
