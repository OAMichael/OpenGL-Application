#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include "../headers/ISceneObject.hpp"
#include "../headers/Mesh.hpp"

namespace Geometry {

class SceneNode : public ISceneObject {

private:
	Mesh mesh_;
	bool isEnabled_ = true;

	glm::mat4 globalMatrix_ = glm::mat4(1.0f);

public:
	std::string name;
	std::vector<SceneNode*> children;
	SceneNode* parent = nullptr;

	bool isLeaf() { return children.empty(); }
	bool isRoot() { return !parent; }

	void setEnabled(bool value = true) { isEnabled_ = value; }
	void setParent(SceneNode* par) { parent = par; }

	void init(const tinygltf::Model& model, const tinygltf::Node& node);
	void draw(GeneralApp::Shader& shader);

	void calculateGlobalModelMatrix();
	glm::mat4 getGlobalModelMatrix() { return globalMatrix_; };
};

}
#endif
