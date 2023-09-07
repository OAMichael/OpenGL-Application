#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <string>

#include <glm/glm.hpp>

#define MAX_NUMBER_OF_LIGHTS 64

namespace Geometry {

struct SceneLight {

	enum LightType : uint32_t {
		POINT_LIGHT = 0,
		DIRECTIONAL_LIGHT = 1,
		SPOT_LIGHT = 2
	};


	std::string name;

	glm::vec4 color;
	glm::vec3 pos_or_dir;

	LightType type;
};

struct LightData {
	glm::vec4 colors[MAX_NUMBER_OF_LIGHTS];
	glm::vec4 pos_or_dir[MAX_NUMBER_OF_LIGHTS];

	uint32_t point_light_num;
	uint32_t directional_light_num;
	uint32_t spot_light_num;
	uint32_t num_of_lights;
};

}

#endif // LIGHT_HPP
