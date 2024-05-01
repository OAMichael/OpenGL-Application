/*
    Common structures and functions for SDF
*/

#ifndef SDF_COMMON_H
#define SDF_COMMON_H

#define M_PI 3.14159265358979323846

struct ObjectDesc {
    float dist;
    uint matID;
};


float getSceneSDF(vec3 pos);
ObjectDesc getSceneSDFMat(vec3 pos);
ObjectDesc getSceneSDFMatOpaque(vec3 pos);
float getSceneSDFAO(vec3 pos);

float dot2(vec2 v) {
    return dot(v, v);
}

float dot2(vec3 v) {
    return dot(v, v);
}


float smin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0);
    return min(a, b) - h * h * 0.25 / k;
}

float smax(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0);
    return max(a, b) + h * h * 0.25 / k;
}

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }
float rand(vec2 co, float l) { return rand(vec2(rand(co), l)); }

float perlin(vec2 p, float dim) {
	vec2 pos = floor(p * dim);
	vec2 posx = pos + vec2(1.0, 0.0);
	vec2 posy = pos + vec2(0.0, 1.0);
	vec2 posxy = pos + vec2(1.0);

	float c = rand(pos, dim);
	float cx = rand(posx, dim);
	float cy = rand(posy, dim);
	float cxy = rand(posxy, dim);

	vec2 d = fract(p * dim);
	d = -0.5 * cos(d * M_PI) + 0.5;

	float ccx = mix(c, cx, d.x);
	float cycxy = mix(cy, cxy, d.x);
	float center = mix(ccx, cycxy, d.y);

	return center * 2.0 - 1.0;
}

#endif