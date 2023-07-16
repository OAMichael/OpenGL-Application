#version 460 core

in vec3 normal;
in vec3 position;
in vec2 texcoord;

uniform sampler2D tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float time;

out vec4 color;

vec3 lightPos = vec3(cos(time), 2.0f, sin(time));


void main() {
	vec3 worldPos = position;
	color = clamp(texture(tex, texcoord) * dot(normal, normalize(lightPos - worldPos)), vec4(0.0f), vec4(1.0f));
}
