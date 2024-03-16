#ifndef TEXTURE_SAMPLING_H
#define TEXTURE_SAMPLING_H

#include "InOutModel.h"


vec4 GetBaseColorSample(const vec2 uv) {
    return texture(uMaterialTextures[TEXTURE_INDEX_BASE_COLOR], uv);
}
vec4 GetBaseColorFactor() {
    return uMaterialTexturesFactors[TEXTURE_INDEX_BASE_COLOR];
}
vec4 GetBaseColorFull(const vec2 uv) {
    return GetBaseColorSample(uv) * GetBaseColorFactor();
}


vec4 GetMetallicRoughnessSample(const vec2 uv) {
    return texture(uMaterialTextures[TEXTURE_INDEX_METALLIC_ROUGHNESS], uv);
}
vec4 GetMetallicRoughnessFactor() {
    return uMaterialTexturesFactors[TEXTURE_INDEX_METALLIC_ROUGHNESS];
}
vec4 GetMetallicRoughnessFull(const vec2 uv) {
    return GetMetallicRoughnessSample(uv) * GetMetallicRoughnessFactor();
}


vec4 GetEmissiveSample(const vec2 uv) {
    return texture(uMaterialTextures[TEXTURE_INDEX_EMISSIVE], uv);
}
vec4 GetEmissiveFactor() {
    return uMaterialTexturesFactors[TEXTURE_INDEX_EMISSIVE];
}
vec4 GetEmissiveFull(const vec2 uv) {
    return GetEmissiveSample(uv) * GetEmissiveFactor();
}


vec4 GetNormalSample(const vec2 uv) {
    return texture(uMaterialTextures[TEXTURE_INDEX_NORMAL], uv);
}
vec4 GetNormalFactor() {
    return uMaterialTexturesFactors[TEXTURE_INDEX_NORMAL];
}
vec4 GetNormalFull(const vec2 uv) {
    return GetNormalSample(uv) * GetNormalFactor();
}


vec4 GetOcclusionSample(const vec2 uv) {
    return texture(uMaterialTextures[TEXTURE_INDEX_OCCLUSION], uv);
}
vec4 GetOcclusionFactor() {
    return uMaterialTexturesFactors[TEXTURE_INDEX_OCCLUSION];
}
vec4 GetOcclusionFull(const vec2 uv) {
    return GetOcclusionSample(uv) * GetOcclusionFactor();
}

#endif