/*
    Common structures and functions for SDF
*/

#ifndef SDF_COMMON_H
#define SDF_COMMON_H

#define M_PI 3.14159265358979323846

uniform sampler2D uRGBANoiseSampler;


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


float rand(float co) {
    return fract(sin(co) * 43758.5453);
}

float rand(vec2 co) {
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand(vec3 co) {
	return fract(sin(dot(co, vec3(12.9898, 78.233, 37.123))) * 43758.5453);
}

float rand(vec2 co, float l) {
	return rand(vec2(rand(co), l));
}


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


float hashNoise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0f - 2.0f * f);

    float n = p.x + p.y * 57.0f + 113.0f * p.z;

    float res = mix(mix(mix(rand(n + 0.0),   rand(n + 1.0), f.x),
                        mix(rand(n + 57.0),  rand(n + 58.0), f.x), f.y),
                    mix(mix(rand(n + 113.0), rand(n + 114.0), f.x),
                        mix(rand(n + 170.0), rand(n + 171.0), f.x), f.y), f.z);
    return res;
}


/*
 *  RGBA Noise functions
 */

vec4 rgbaNoiseSample(vec2 uv) {
	return textureLod(uRGBANoiseSampler, uv, 0.0);
}


float rgbaNoise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    vec2 uv = ((p.xy + vec2(37.0, 17.0) * p.z) + f.xy + 0.5f) / 256.0f;
    vec2 rg = rgbaNoiseSample(uv).xy;
    return mix(rg.x, rg.y, f.z);
}


/*
 *  Fractional Brownian Motion functions
 */

float fbm2(vec3 pos) {
    float totalNoise = 0.0;
    totalNoise += 1.0 * rgbaNoise(pos * 1.0);
    totalNoise += 0.5 * rgbaNoise(pos * 2.0);
    return totalNoise;
}

float fbm3(vec3 pos) {
    float totalNoise = 0.0;
    totalNoise += 1.00 * rgbaNoise(pos * 1.0);
    totalNoise += 0.50 * rgbaNoise(pos * 2.0);
    totalNoise += 0.25 * rgbaNoise(pos * 4.0);
    return totalNoise;
}

float fbm4(vec3 pos) {
    float totalNoise = 0.0;
    totalNoise += 1.000 * rgbaNoise(pos * 1.0);
    totalNoise += 0.500 * rgbaNoise(pos * 2.0);
    totalNoise += 0.250 * rgbaNoise(pos * 4.0);
    totalNoise += 0.125 * rgbaNoise(pos * 8.0);
    return totalNoise;
}

float fbm5(vec3 pos) {
    float totalNoise = 0.0;
    totalNoise += 1.0000 * rgbaNoise(pos * 1.0);
    totalNoise += 0.5000 * rgbaNoise(pos * 2.0);
    totalNoise += 0.2500 * rgbaNoise(pos * 4.0);
    totalNoise += 0.1250 * rgbaNoise(pos * 8.0);
    totalNoise += 0.0625 * rgbaNoise(pos * 16.0);
    return totalNoise;
}

float fbm6(vec3 pos) {
    float totalNoise = 0.0;
    totalNoise += 1.00000 * rgbaNoise(pos * 1.0);
    totalNoise += 0.50000 * rgbaNoise(pos * 2.0);
    totalNoise += 0.25000 * rgbaNoise(pos * 4.0);
    totalNoise += 0.12500 * rgbaNoise(pos * 8.0);
    totalNoise += 0.06250 * rgbaNoise(pos * 16.0);
    totalNoise += 0.03125 * rgbaNoise(pos * 32.0);
    return totalNoise;
}

float fbmN(vec3 pos, uint N) {
    float totalNoise = 0.0;
    float factor = 1.0f;
    for (uint i = 0; i < N; i++) {
        totalNoise += (1.0f / factor) * rgbaNoise(pos * factor);
        factor *= 2.0f;
    }
    return totalNoise;
}


float fbm2Mat(vec3 pos, mat3 mat) {
    float totalNoise = 0.0;
    totalNoise += 0.50 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.25 * hashNoise(pos);
    return totalNoise;
}

float fbm3Mat(vec3 pos, mat3 mat) {
    float totalNoise = 0.0;
    totalNoise += 0.500 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.250 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.125 * hashNoise(pos);
    return totalNoise;
}

float fbm4Mat(vec3 pos, mat3 mat) {
    float totalNoise = 0.0;
    totalNoise += 0.5000 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.2500 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.1250 * hashNoise(pos);
    pos = mat * pos * 2.0;
    totalNoise += 0.0625 * hashNoise(pos);
    return totalNoise;
}

#endif
