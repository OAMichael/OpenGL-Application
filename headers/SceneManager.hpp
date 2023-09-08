#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <unordered_map>

#include "SceneNode.hpp"
#include "Cube.hpp"
#include "Light.hpp"

namespace SceneResources {

inline constexpr const char* ENVIRONMENT_SHADER_NAME		= "Default_Environment";
inline constexpr const char* BACKGROUND_2D_TEXTURE_NAME		= "BACKGROUND_2D_TEXTURE";
inline constexpr const char* SKYBOX_TEXTURE_NAME			= "SKYBOX_TEXTURE";
inline constexpr const char* EQUIRECTANGULAR_TEXTURE_NAME	= "EQUIRECTANGULAR_TEXTURE";


struct LightDesc {
	std::string name = "";

	glm::vec4 color = glm::vec4(0.0);
	glm::vec3 position = glm::vec3(0.0);
	glm::vec3 direction = glm::vec3(0.0);
	float cutoff_angle = 0.0;

	SceneLight::LightType type;
};

class SceneManager final {
public:
	enum EnvironmentType : uint32_t {
		BACKGROUND_IMAGE_2D = 0,
		SKYBOX,
		EQUIRECTANGULAR
	};

	SceneManager(const SceneManager& obj) = delete;

	static SceneManager* getInstance() {
		if (!instancePtr)
			instancePtr = new SceneManager();

		return instancePtr;
	}

	static constexpr const char* ROOT_NODE_NAME = "RootNode";

	SceneNode& createRootNode();
	SceneNode& getRootNode();

	SceneNode& createSceneNode(const std::string& name);
	SceneLight& createSceneLight(const LightDesc& lightDesc);

	void deleteSceneNode(const SceneHandle handle);
	void deleteSceneLight(const SceneHandle handle);

	SceneNode& getSceneNode(const SceneHandle handle);
	SceneLight& getSceneLight(const SceneHandle handle);

	bool hasSceneNode(const std::string& name);
	bool hasSceneLight(const std::string& name);

	void updateLights();
	const LightData& getLightData() const;

	void createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames);
	void createEnvironment(const EnvironmentType envType, const std::string& textureName);
	void drawEnvironment();

	void createBackground2D(const std::string& textureName);
	void drawBackground2D();

	void createSkybox(const std::vector<std::string>& textureNames);
	void drawSkybox();

	void createEquirectangular(const std::string& textureName);
	void drawEquirectangular();

	EnvironmentType getEnvironmentType();

	const Resources::ResourceHandle getBackground2DHandle() const;
	const Resources::ResourceHandle getSkyboxHandle() const;
	const Resources::ResourceHandle getEquirectangularHandle() const;

	~SceneManager();

private:
	std::unordered_map<SceneHandle, SceneNode*> sceneNodes_;
	std::unordered_map<SceneHandle, SceneLight*> sceneLights_;

	LightData lightData_;

	std::unordered_map<SceneHandle, ISceneObject*> sceneObjects_;

	SceneNode* rootNode_ = nullptr;

	unsigned VAOBackground2D_;
	unsigned VBOBackground2D_;
	Resources::ResourceHandle Background2DHandle_;

	unsigned VAOSkybox_;
	unsigned VBOSkybox_;
	Geometry::Cube Skybox_{glm::vec3(0.0f)};
	Resources::ResourceHandle SkyboxHandle_;

	unsigned VAOEquirect_;
	unsigned VBOEquirect_;
	Geometry::Cube Equirect_{glm::vec3(0.0f)};
	Resources::ResourceHandle EquirectHandle_;

	EnvironmentType envType_;

	static SceneManager* instancePtr;
	SceneManager();
};

}

#endif // SCENE_MANAGER_HPP