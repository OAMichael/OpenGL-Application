#ifndef CONSTANTS_H
#define CONSTANTS_H

#define TEXTURE_INDEX_BASE_COLOR            0
#define TEXTURE_INDEX_METALLIC_ROUGHNESS    1
#define TEXTURE_INDEX_EMISSIVE              2
#define TEXTURE_INDEX_NORMAL                3
#define TEXTURE_INDEX_OCCLUSION             4

#define TEXTURE_INDEX_COUNT                 5


#define MATERIAL_FLAG_NORMAL_MAP_BIT        1 << 0

#define	BACKGROUND_IMAGE_2D 0
#define	SKYBOX 1
#define	EQUIRECTANGULAR 2

#define PI 3.14159


#define POINT_LIGHT          0
#define DIRECTIONAL_LIGHT    1
#define SPOT_LIGHT           2

#define MAX_NUMBER_OF_LIGHTS 64


struct LightData {
	vec4 colors[MAX_NUMBER_OF_LIGHTS];
	vec4 positions[MAX_NUMBER_OF_LIGHTS];
	vec4 direction_cutoffs[MAX_NUMBER_OF_LIGHTS];

	uint point_light_offset;
	uint directional_light_offset;
	uint spot_light_offset;
	uint num_of_lights;
};


vec2 CubemapToEquirect(vec3 v) {
	const vec2 invAtan = vec2(0.1591, 0.3183);
	vec2 uv = vec2(atan(v.z, v.x), -asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}


// D component
float GGX_Trowbridge_Reitz(const float alpha, const vec3 N, const vec3 H) {
    const float alpha2 = pow(alpha, 2.0f);
    const float num = alpha2;

    const float NoH = max(dot(N, H), 0.0f);
    float den = PI * pow(pow(NoH, 2.0f) * (alpha2 - 1.0) + 1.0, 2.0f);
    den = max(den, 0.000001f);

    return num / den;
}

// Schlick - Beckmann
float G1_Schlick_Beckmann(const float alpha, const vec3 N, const vec3 X) {
    const float NoX = max(dot(N, X), 0.0f);
    const float num = NoX;

    const float k = alpha / 2.0f;
    float den = NoX * (1 - k) + k;
    den = max(den, 0.000001f);

    return num / den;
}

// Schlick_Beckmann full G function
float G_Schlick_Beckmann(const float alpha, const vec3 N, const vec3 V, const vec3 L) {
    return G1_Schlick_Beckmann(alpha, N, V) * G1_Schlick_Beckmann(alpha, N, L);
}

// Fresnel - Schlick 
vec3 Fresnel_Schlick(const vec3 F0, const vec3 V, const vec3 H) {
    return F0 + (vec3(1.0f) - F0) * pow(1 - max(dot(V, H), 0.0f), 5.0f);
}

vec3  Fresnel_Schlick_Roughness(const vec3 F0, const vec3 V, const vec3 H, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1 - max(dot(V, H), 0.0f), 5.0f);
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N) {
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
#endif