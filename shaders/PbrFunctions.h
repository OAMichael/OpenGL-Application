#include "TextureSampling.h"


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
    vec3 normalMap = GetNormalSample(uv).xyz;
    const float normalScale = GetNormalFactor().x;
    normalMap = (normalMap * 2.0 - 1.0) * vec3(normalScale, normalScale, 1.0f);
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

    vec3 baseColor = GetBaseColorFull(inUv).rgb;
    baseReflectivity = mix(baseReflectivity, baseColor, metallic);

    const float alpha = pow(roghness, 2.0f);

    const vec3 Ks = Fresnel_Schlick(baseReflectivity, V, H);
    const vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

    const vec3 diffuseColor = baseColor / PI;

    //                                     D                              G                               F
    const vec3 CookTorranceNum = GGX_Trowbridge_Reitz(alpha, N, H) * G_Schlick_Beckmann(alpha, N, V, L) * Ks;
    float CookTorranceDen = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
    CookTorranceDen = max(CookTorranceDen, 0.000001f);

    const vec3 CookTorrance = CookTorranceNum / CookTorranceDen;

    return Kd * diffuseColor + CookTorrance;
}



vec4 pbrBasic() {
    vec4 color = vec4(0.0f);
    const vec3 worldPos = inPosition;

    vec3 N = normalize(inNormal);
    const vec3 V = normalize(cameraWorldPos - worldPos);

    if ((materialFlags & MATERIAL_FLAG_NORMAL_MAP_BIT) == MATERIAL_FLAG_NORMAL_MAP_BIT) {
        N = applyNormalMap(N, V, inUv);
    }

    for (uint i = 0; i < lights.point_light_num; i++) {
        const vec3 lightBaseColor = lights.colors[i].rgb;
        const float lightDist = length(worldPos - lights.pos_or_dir[i].xyz);
        const float attenuation = 1 / lightDist / lightDist;

        const vec3 lightColor = lightBaseColor * attenuation;
        const vec3 L = normalize(lights.pos_or_dir[i].xyz - worldPos);
        const float NoL = max(dot(N, L), 0.0f);

        const vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.point_light_num; i < lights.point_light_num + lights.directional_light_num; i++) {
        const vec3 lightColor = lights.colors[i].rgb;
        const vec3 L = normalize(lights.pos_or_dir[i].xyz);
        const float NoL = max(dot(N, L), 0.0f);

        const vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.point_light_num + lights.directional_light_num; i < lights.point_light_num + lights.directional_light_num + lights.spot_light_num; i++) {

    }

    vec3 baseReflectivity = vec3(0.04);

    const vec2 metallicRoughness = GetMetallicRoughnessFull(inUv).zy;
    const float metallic = metallicRoughness.x;
    const float roghness = metallicRoughness.y;

    vec3 baseColor = GetBaseColorFull(inUv).rgb;
    baseReflectivity = mix(baseReflectivity, baseColor, metallic);

    const vec3 Ks = Fresnel_Schlick(baseReflectivity, V, N);
    const vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

    vec3 envSample = vec3(1.0f);
    if (environmentType == SKYBOX) {
        envSample = textureCubeLod(uCubeSamplerSkybox, reflect(-V, N), roghness).rgb;
    }
    else if (environmentType == EQUIRECTANGULAR) {
        const vec3 normUv = normalize(reflect(-V, N));
        const float phi = atan(normUv.z, normUv.x);
        const float psi = asin(-normUv.y);
        const float u = phi / 2.0 / PI;
        const float v = 0.5f + 1.0 / PI * psi;

        envSample = textureLod(uSamplerEquirect, vec2(u, v), roghness).rgb;
    }

    const vec3 diffuse = envSample * baseColor;

    const float ao = GetOcclusionFull(inUv).x;

    const vec3 ambient = Kd * diffuse * ao;
    color.rgb += ambient;

    const vec3 emissive = GetEmissiveFull(inUv).rgb;
    color.rgb += emissive;

    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    return color;
}