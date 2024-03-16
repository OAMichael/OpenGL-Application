#include "TextureSampling.h"


mat3 calculateTBN(const vec3 N, const vec3 position, const vec2 uv)
{
    vec3 dp1 = dFdx(position);
    vec3 dp2 = dFdy(position);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}


vec3 applyNormalMap(const vec3 N, const vec3 V, const vec2 uv)
{
    vec3 normalMap = GetNormalSample(uv).xyz;
    float normalScale = GetNormalFactor().x;
    normalMap = (normalMap * 2.0 - 1.0) * vec3(normalScale, normalScale, 1.0f);
    mat3 TBN = calculateTBN(N, -V, uv);
    return normalize(TBN * normalMap);
}



vec3 calculateBRDF(const vec3 N, const vec3 L, const vec3 V) {
    vec3 baseReflectivity = vec3(0.04);
    vec3 H = normalize(L + V);

    vec2 metallicRoughness = GetMetallicRoughnessFull(inUv).zy;
    float metallic = metallicRoughness.x;
    float roghness = metallicRoughness.y;

    vec3 baseColor = GetBaseColorFull(inUv).rgb;
    baseReflectivity = mix(baseReflectivity, baseColor, metallic);

    float alpha = pow(roghness, 2.0f);

    vec3 Ks = Fresnel_Schlick(baseReflectivity, V, H);
    vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

    vec3 diffuseColor = baseColor / PI;

    //                               D                              G                               F
    vec3 CookTorranceNum = GGX_Trowbridge_Reitz(alpha, N, H) * G_Schlick_Beckmann(alpha, N, V, L) * Ks;
    float CookTorranceDen = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
    CookTorranceDen = max(CookTorranceDen, 0.000001f);

    vec3 CookTorrance = CookTorranceNum / CookTorranceDen;

    return Kd * diffuseColor + CookTorrance;
}


vec4 pbrBasic() {
    vec4 color = vec4(0.0f);
    vec3 worldPos = inPosition;

    vec3 N = normalize(inNormal);
    vec3 V = normalize(uCameraWorldPos - worldPos);
    vec3 R = reflect(-V, N);

    bool hasNormalMap = bool(uMaterialFlags & MATERIAL_FLAG_NORMAL_MAP_BIT);
    if (hasNormalMap) {
        N = applyNormalMap(N, V, inUv);
    }

    for (uint i = lights.point_light_offset; i < lights.directional_light_offset; i++) {
        vec3 lightBaseColor = lights.colors[i].rgb;
        float lightDist = length(worldPos - lights.positions[i].xyz);
        float attenuation = 1.0f / lightDist / lightDist;

        vec3 lightColor = lightBaseColor * attenuation;
        vec3 L = normalize(lights.positions[i].xyz - worldPos);
        float NoL = max(dot(N, L), 0.0f);

        vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.directional_light_offset; i < lights.spot_light_offset; i++) {
        vec3 lightColor = lights.colors[i].rgb;
        vec3 L = normalize(lights.direction_cutoffs[i].xyz);
        float NoL = max(dot(N, L), 0.0f);

        vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.spot_light_offset; i < lights.num_of_lights; i++) {
        vec3 L = normalize(lights.positions[i].xyz - worldPos);
        float cos_angle = dot(-L, normalize(lights.direction_cutoffs[i].xyz));
        if (cos_angle > lights.direction_cutoffs[i].w) {
            vec3 lightBaseColor = lights.colors[i].rgb;
            float lightDist = length(worldPos - lights.positions[i].xyz);
            float attenuation = 1.0f / lightDist / lightDist;

            vec3 lightColor = lightBaseColor * attenuation;
            float NoL = max(dot(N, L), 0.0f);

            vec3 brdf = calculateBRDF(N, L, V);
            color.rgb += brdf * lightColor * NoL;
        }
    }

    vec3 baseReflectivity = vec3(0.04);

    vec2 metallicRoughness = GetMetallicRoughnessFull(inUv).zy;
    float metallic = metallicRoughness.x;
    float roghness = metallicRoughness.y;

    vec3 baseColor = GetBaseColorFull(inUv).rgb;
    baseReflectivity = mix(baseReflectivity, baseColor, metallic);

    vec3 Ks = Fresnel_Schlick_Roughness(baseReflectivity, V, N, roghness);
    vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

    vec3 irradiance = vec3(1.0f);
    if (uEnvironmentType == SKYBOX || uEnvironmentType == EQUIRECTANGULAR) {
        irradiance = texture(uIrradianceMap, normalize(N)).rgb;
    }

    vec3 diffuse = irradiance * baseColor;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(uPrefilterMap, R, roghness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(uBrdfLUT, vec2(max(dot(N, V), 0.0), roghness)).rg;
    vec3 specular = prefilteredColor * (Ks * brdf.x + brdf.y);

    float ao = GetOcclusionFull(inUv).x;

    vec3 ambient = (Kd * diffuse + specular) * ao;
    color.rgb += ambient;

    vec3 emissive = GetEmissiveFull(inUv).rgb;
    color.rgb += emissive;

    return color;
}