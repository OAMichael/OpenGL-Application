#version 460 core
layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

uniform sampler2D uSceneColor;
uniform sampler2D uBloomBlur;


void main() {
	vec3 color = texture(uSceneColor, inUv).rgb;
	vec3 brightColor = texture(uBloomBlur, inUv).rgb;
	float brightness = dot(brightColor, vec3(0.2126, 0.7152, 0.0722));
	outColor.rgb = color + clamp(brightness - 1, 0.0, 1.0) * brightColor.rgb;
	outColor.a = 1.0;
}