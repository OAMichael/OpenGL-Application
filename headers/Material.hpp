#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "../headers/Texture.hpp"

namespace Resources {

struct Material {

	enum TextureIdx : uint32_t {
		BASE_COLOR = 0,
		METALLIC_ROUGHNESS = 1,
		EMISSIVE = 2,
		NORMAL = 3,
		OCCLUSION = 4,

		COUNT
	};

	std::string name;
	Texture* textures[TextureIdx::COUNT];

	enum DefaultMaterials : uint32_t {
		DEFAULT_MATERIAL = 0,
	};
};


extern std::string defaultMaterialName;

}
#endif
