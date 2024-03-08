#version 460 core

#include "../Constants.h"

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 outColor;


uniform samplerCube uSamplerSkybox;
uniform sampler2D uSamplerEquirect;

uniform uint uEnvironmentType;
uniform float uRoughness;


void main() {
    vec3 N = normalize(inPos);
    
    vec3 R = N;
    vec3 V = R;

    const bool isEquirect = uEnvironmentType == EQUIRECTANGULAR;
    const uint SAMPLE_COUNT = 512u;

    float totalArea = 1.0f;
    if(isEquirect) {
        ivec2 size = textureSize(uSamplerEquirect, 0);
        totalArea = size.x * size.y;
    }
    else {
        float size = textureSize(uSamplerSkybox, 0).x;
        totalArea = 6.0f * size * size;
    }

    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, uRoughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) {
            continue;
        }

        float D = GGX_Trowbridge_Reitz(uRoughness * uRoughness, N, H);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

        float saTexel  = 4.0 * PI / totalArea;
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
        float mipLevel = uRoughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            
        if(isEquirect) {
            prefilteredColor += textureLod(uSamplerEquirect, CubemapToEquirect(L), mipLevel).rgb * NdotL;                
        }
        else {
            prefilteredColor += textureLod(uSamplerSkybox, L, mipLevel).rgb * NdotL;
        }
        totalWeight += NdotL;
    }

    prefilteredColor = prefilteredColor / totalWeight;
    outColor = vec4(prefilteredColor, 1.0);
}
