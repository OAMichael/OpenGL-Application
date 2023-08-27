#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <unordered_map>

#include "../headers/SceneNode.hpp"
#include "../headers/Cube.hpp"

namespace SceneResources {

class SceneManager final {
public:
	enum EnvironmentType : uint32_t {
		BACKGROUND_IMAGE_2D = 0,
		SKYBOX
	};

	SceneManager(const SceneManager& obj) = delete;

	static SceneManager* getInstance() {
		if (!instancePtr)
			instancePtr = new SceneManager();

		return instancePtr;
	}

	Geometry::SceneNode& createSceneNode(const std::string& name);

	void deleteSceneNode(const std::string& name);

	Geometry::SceneNode& getSceneNode(const std::string& name);

	bool hasSceneNode(const std::string& name);

	void createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames);
	void createEnvironment(const EnvironmentType envType, const std::string& textureName);
	void drawEnvironment();

	void createSkybox(const std::vector<std::string>& textureNames);
	void drawSkybox();

	void createBackground2D(const std::string& textureName);
	void drawBackground2D();


	~SceneManager();

private:
	std::unordered_map<std::string, Geometry::SceneNode*> sceneNodes_;

	unsigned VAOSkybox_;
	unsigned VBOSkybox_;
	Geometry::Cube Skybox_{glm::vec3(0.0f)};

	unsigned VAOBackground2D_;
	unsigned VBOBackground2D_;

	GeneralApp::Shader defaultEnvShader_;
	EnvironmentType envType_;

	static SceneManager* instancePtr;
	SceneManager();
};

}

#endif // SCENE_MANAGER_HPP