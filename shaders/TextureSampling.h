#ifndef TEXTURE_SAMPLING_H
#define TEXTURE_SAMPLING_H

#include "InOutModel.h"

#define CONSTRUCT_ALL_SAMPLING(name, idx)       vec4 Get##name##Sample(const vec2 uv) {                 \
                                                    return texture(materialTextures[idx], uv);          \
                                                }                                                       \
                                                vec4 Get##name##Factor() {                              \
                                                    return materialTexturesFactors[idx];                \
                                                }                                                       \
                                                vec4 Get##name##Full(const vec2 uv) {                   \
                                                    return Get##name##Sample(uv) * Get##name##Factor(); \
                                                }

CONSTRUCT_ALL_SAMPLING(BaseColor, TEXTURE_INDEX_BASE_COLOR)
CONSTRUCT_ALL_SAMPLING(MetallicRoughness, TEXTURE_INDEX_METALLIC_ROUGHNESS)
CONSTRUCT_ALL_SAMPLING(Emissive, TEXTURE_INDEX_EMISSIVE)
CONSTRUCT_ALL_SAMPLING(Normal, TEXTURE_INDEX_NORMAL)
CONSTRUCT_ALL_SAMPLING(Occlusion, TEXTURE_INDEX_OCCLUSION)


#endif