#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Texture.hpp"

#include <RenderResource.hpp>

namespace Resources {

struct Material : RenderResource {

	enum TextureIdx : uint32_t {
		BASE_COLOR = 0,
		METALLIC_ROUGHNESS = 1,
		EMISSIVE = 2,
		NORMAL = 3,
		OCCLUSION = 4,

		COUNT
	};

	enum MaterialFlags : uint32_t {
		MATERIAL_FLAG_NORMAL_MAP_BIT = 1 << 0
	};

	uint32_t materialFlags;

	Texture* textures[TextureIdx::COUNT];

	enum DefaultMaterials : uint32_t {
		DEFAULT_MATERIAL = 0,
	};
};


extern std::string defaultMaterialName;

}
#endif
