#version 460 core

#define TEXTURE_INDEX_BASE_COLOR			0
#define TEXTURE_INDEX_METALLIC_ROUGHNESS	1
#define TEXTURE_INDEX_EMISSIVE				2
#define TEXTURE_INDEX_NORMAL				3
#define TEXTURE_INDEX_OCCLUSION				4

#define TEXTURE_INDEX_COUNT					5

#define MATERIAL_FLAG_NORMAL_MAP_BIT		1 << 0

#define PI 3.14159


uniform sampler2D materialTextures[TEXTURE_INDEX_COUNT];
uniform vec4 materialTexturesFactors[TEXTURE_INDEX_COUNT];

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inUv;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 proj;
    mat4 model;
};

uniform vec3 cameraWorldPos;
uniform uint materialFlags;

#define CONSTRUCT_ALL_SAMPLING(name, idx)		vec4 Get##name##Sample(const vec2 uv) {					\
													return texture(materialTextures[idx], uv);			\
												}														\
												vec4 Get##name##Factor() {								\
													return materialTexturesFactors[idx];				\
												}														\
												vec4 Get##name##Full(const vec2 uv) {					\
													return Get##name##Sample(uv) * Get##name##Factor();	\
												}

CONSTRUCT_ALL_SAMPLING(BaseColor, TEXTURE_INDEX_BASE_COLOR)
CONSTRUCT_ALL_SAMPLING(MetallicRoughness, TEXTURE_INDEX_METALLIC_ROUGHNESS)
CONSTRUCT_ALL_SAMPLING(Emissive, TEXTURE_INDEX_EMISSIVE)
CONSTRUCT_ALL_SAMPLING(Normal, TEXTURE_INDEX_NORMAL)
CONSTRUCT_ALL_SAMPLING(Occlusion, TEXTURE_INDEX_OCCLUSION)


uniform float time;

out vec4 outColor;

mat3 calculateTBN(const vec3 N, const vec3 position, const vec2 uv)
{
    const vec3 dp1 = dFdx(position);
    const vec3 dp2 = dFdy(position);
    const vec2 duv1 = dFdx(uv);
    const vec2 duv2 = dFdy(uv);
 
    const vec3 dp2perp = cross(dp2, N);
    const vec3 dp1perp = cross(N, dp1);
    const vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    const vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    const float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}


vec3 applyNormalMap(const vec3 N, const vec3 V, const vec2 uv)
{
    vec3 normalMap = GetNormalFull(uv).xyz;
    normalMap = normalMap * 2.0 - 1.0;
    const mat3 TBN = calculateTBN(N, -V, uv);
    return normalize(TBN * normalMap);
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


vec3 calculateBRDF(const vec3 N, const vec3 L, const vec3 V) {
	vec3 baseReflectivity = vec3(0.04);
	const vec3 H = normalize(L + V);
	
	const vec2 metallicRoughness = GetMetallicRoughnessFull(inUv).zy;
	const float metallic = metallicRoughness.x;
	const float roghness = metallicRoughness.y;

	const vec3 baseColor = GetBaseColorFull(inUv).rgb;
	baseReflectivity = mix(baseReflectivity, baseColor, metallic);

	const float alpha = pow(roghness, 2.0f);
	
	const vec3 Ks = Fresnel_Schlick(baseReflectivity, V, H);
	const vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

	const vec3 diffuseColor = baseColor / PI;

	//								D									G							F
	const vec3 CookTorranceNum = GGX_Trowbridge_Reitz(alpha, N, H) * G_Schlick_Beckmann(alpha, N, V, L) * Ks;
	float CookTorranceDen = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
	CookTorranceDen = max(CookTorranceDen, 0.000001f);

	const vec3 CookTorrance = CookTorranceNum / CookTorranceDen;

	return Kd * diffuseColor + CookTorrance;
}



vec4 pbrBasic() {
	vec4 color = vec4(0.0f);
	const vec3 worldPos = inPosition;
	const vec3 lightBaseColor = vec3(1.0f);
	
	vec3 N = normalize(inNormal);
	const vec3 V = normalize(cameraWorldPos - worldPos);
	
	if((materialFlags & MATERIAL_FLAG_NORMAL_MAP_BIT) == MATERIAL_FLAG_NORMAL_MAP_BIT) {
		N = applyNormalMap(N, V, inUv);
	}

	const vec3 pointLightPositions[4] = { 
		2.0f * vec3(cos(0.1*time), 1.0f, sin(0.1*time)),
		2.0f * vec3(sin(0.1*time), 1.0f, cos(0.1*time)),
		2.0f * vec3(cos(0.1*time), 1.0f, -sin(0.1*time)),
		2.0f * vec3(sin(0.1*time), 1.0f, -cos(0.1*time))
	};

	const vec3 directionalLightDirs[4] = { 
		vec3(cos(time), 0.0f, sin(time)),
		vec3(sin(time), 0.0f, cos(time)),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	};
	
	for(int i = 0; i < 4; i++) {
		const float lightDist = length(worldPos - pointLightPositions[i]);
		const float attenuation = 1 / lightDist / lightDist;

		const vec3 lightColor = lightBaseColor * attenuation;
		const vec3 L = normalize(pointLightPositions[i] - worldPos);
		const float NoL = max(dot(N, L), 0.0f);

		const vec3 brdf = calculateBRDF(N, L, V);
		color.rgb += brdf * lightColor * NoL;
	}

	for(int i = 0; i < 4; i++) {
		const vec3 lightColor = lightBaseColor;	
		const vec3 L = normalize(directionalLightDirs[i]);
		const float NoL = max(dot(N, L), 0.0f);

		const vec3 brdf = calculateBRDF(N, L, V);
		color.rgb += brdf * lightColor * NoL;
	}

	const float ao = GetOcclusionFull(inUv).x;
	const vec3 baseColor = GetBaseColorFull(inUv).rgb;

	const vec3 ambient = vec3(0.03) * baseColor * ao;
	color.rgb += ambient;  

	return color;
}


void main() {
	outColor = pbrBasic();
}
