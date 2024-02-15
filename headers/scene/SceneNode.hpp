#ifndef SCENENODE_HPP
#define SCENENODE_HPP

#include "ISceneObject.hpp"
#include "Mesh.hpp"

namespace SceneResources {

class SceneNode : public ISceneObject {

private:
	Geometry::Mesh mesh_;
	bool isEnabled_ = true;

	glm::mat4 globalMatrix_ = glm::mat4(1.0f);

public:
	std::string name;
	std::vector<SceneNode*> children;
	SceneNode* parent = nullptr;

	bool isLeaf() { return children.empty(); }
	bool isRoot() { return !parent; }

	void setEnabled(bool value = true) { isEnabled_ = value; }
	void setParent(SceneNode* par);

	void init(const tinygltf::Model& model, const tinygltf::Node& node);
	void draw(GeneralApp::Shader& shader);

	void calculateGlobalModelMatrix();
	glm::mat4 getGlobalModelMatrix() { return globalMatrix_; };

	void printNode(const int level = 0);
};

}
#endif
