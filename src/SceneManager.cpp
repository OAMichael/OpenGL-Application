#include "../headers/SceneManager.hpp"


namespace SceneResources {

SceneManager* SceneManager::instancePtr = nullptr;

Geometry::SceneNode& SceneManager::createSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end())
        return *sceneNodes_[name];

    Geometry::SceneNode* newNode = new Geometry::SceneNode();

#ifdef DEBUG_MODEL
    std::cout << "Creating scene node \'" << name << "\'" << std::endl;
#endif

    newNode->name = name;
    sceneNodes_[newNode->name] = newNode;

    return *newNode;
}

void SceneManager::deleteSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end()) {
        delete sceneNodes_[name];
        sceneNodes_.erase(it);
    }
}

Geometry::SceneNode& SceneManager::getSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end())
        return *sceneNodes_[name];

    auto emptyNode = new Geometry::SceneNode();
    sceneNodes_[name] = emptyNode;
    return *emptyNode;
}

bool SceneManager::hasSceneNode(const std::string& name) {
    auto it = sceneNodes_.find(name);
    return it != sceneNodes_.end();
}


SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
    for (auto& node : sceneNodes_) {
        delete node.second;
        sceneNodes_.erase(node.first);
    }
}
}
