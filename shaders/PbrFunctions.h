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
    const vec3 V = normalize(uCameraWorldPos - worldPos);
    vec3 R = reflect(-V, N);

    if ((uMaterialFlags & MATERIAL_FLAG_NORMAL_MAP_BIT) == MATERIAL_FLAG_NORMAL_MAP_BIT) {
        N = applyNormalMap(N, V, inUv);
    }

    for (uint i = lights.point_light_offset; i < lights.directional_light_offset; i++) {
        const vec3 lightBaseColor = lights.colors[i].rgb;
        const float lightDist = length(worldPos - lights.positions[i].xyz);
        const float attenuation = 1 / lightDist / lightDist;

        const vec3 lightColor = lightBaseColor * attenuation;
        const vec3 L = normalize(lights.positions[i].xyz - worldPos);
        const float NoL = max(dot(N, L), 0.0f);

        const vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.directional_light_offset; i < lights.spot_light_offset; i++) {
        const vec3 lightColor = lights.colors[i].rgb;
        const vec3 L = normalize(lights.direction_cutoffs[i].xyz);
        const float NoL = max(dot(N, L), 0.0f);

        const vec3 brdf = calculateBRDF(N, L, V);
        color.rgb += brdf * lightColor * NoL;
    }

    for (uint i = lights.spot_light_offset; i < lights.num_of_lights; i++) {
        const vec3 L = normalize(lights.positions[i].xyz - worldPos);
        const float cos_angle = dot(-L, normalize(lights.direction_cutoffs[i].xyz));
        if (cos_angle > lights.direction_cutoffs[i].w) {
            const vec3 lightBaseColor = lights.colors[i].rgb;
            const float lightDist = length(worldPos - lights.positions[i].xyz);
            const float attenuation = 1 / lightDist / lightDist;

            const vec3 lightColor = lightBaseColor * attenuation;
            const float NoL = max(dot(N, L), 0.0f);

            const vec3 brdf = calculateBRDF(N, L, V);
            color.rgb += brdf * lightColor * NoL;
        }
    }

    vec3 baseReflectivity = vec3(0.04);

    const vec2 metallicRoughness = GetMetallicRoughnessFull(inUv).zy;
    const float metallic = metallicRoughness.x;
    const float roghness = metallicRoughness.y;

    vec3 baseColor = GetBaseColorFull(inUv).rgb;
    baseReflectivity = mix(baseReflectivity, baseColor, metallic);

    const vec3 Ks = Fresnel_Schlick_Roughness(baseReflectivity, V, N, roghness);
    const vec3 Kd = (1.0f - Ks) * (1.0f - metallic);

    vec3 irradiance = vec3(1.0f);
    if (uEnvironmentType == SKYBOX || uEnvironmentType == EQUIRECTANGULAR) {
        irradiance = texture(uIrradianceMap, normalize(N)).rgb;
    }

    const vec3 diffuse = irradiance * baseColor;

    const float MAX_REFLECTION_LOD = 4.0;
    const vec3 prefilteredColor = textureLod(uPrefilterMap, R, roghness * MAX_REFLECTION_LOD).rgb;
    const vec2 brdf = texture(uBrdfLUT, vec2(max(dot(N, V), 0.0), roghness)).rg;
    const vec3 specular = prefilteredColor * (Ks * brdf.x + brdf.y);

    const float ao = GetOcclusionFull(inUv).x;

    const vec3 ambient = (Kd * diffuse + specular) * ao;
    color.rgb += ambient;

    const vec3 emissive = GetEmissiveFull(inUv).rgb;
    color.rgb += emissive;

    return color;
}