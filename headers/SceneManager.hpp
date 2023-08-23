#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <unordered_map>

#include "../headers/SceneNode.hpp"

namespace SceneResources {

class SceneManager final {
private:
	std::unordered_map<std::string, Geometry::SceneNode*> sceneNodes_;

	static SceneManager* instancePtr;
	SceneManager();

public:

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


	~SceneManager();
};

}

#endif // SCENE_MANAGER_HPP