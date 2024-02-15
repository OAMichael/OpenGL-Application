#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <string>

#include <glm/glm.hpp>

#include "ISceneObject.hpp"


#define MAX_NUMBER_OF_LIGHTS 64

namespace SceneResources {

struct SceneLight : public ISceneObject {

	enum LightType : uint32_t {
		POINT_LIGHT = 0,
		DIRECTIONAL_LIGHT = 1,
		SPOT_LIGHT = 2
	};


	std::string name;

	glm::vec4 color;
	glm::vec3 position;
	glm::vec3 direction;
	float cutoff_angle;

	LightType type;
};

struct LightData {
	glm::vec4 colors[MAX_NUMBER_OF_LIGHTS];
	glm::vec4 positions[MAX_NUMBER_OF_LIGHTS];
	glm::vec4 direction_cutoffs[MAX_NUMBER_OF_LIGHTS];

	uint32_t point_light_offset;
	uint32_t directional_light_offset;
	uint32_t spot_light_offset;
	uint32_t num_of_lights;
};

}

#endif // LIGHT_HPP
