#include "../headers/SceneNode.hpp"



void Geometry::SceneNode::init(const tinygltf::Model& model, const tinygltf::Node& node) {
	if (node.mesh > -1 && node.mesh < model.meshes.size()) {
		mesh_ = Mesh(model, model.meshes[node.mesh]);
		mesh_.init();
	}

	name = node.name;

	for (auto& child : node.children) {
		SceneNode* childNode = new SceneNode();
		childNode->parent = this;
		childNode->init(model, model.nodes[child]);

		children.push_back(childNode);
	}

	auto& translation = node.translation;
	if(translation.size() == 3)
		this->setPosition(glm::vec3(translation[0], translation[1], translation[2]));

	auto& rotation = node.rotation;
	if (rotation.size() == 4) {
		this->setRotation(glm::quat(rotation[3], rotation[0], rotation[1], rotation[2]));		
	}

	auto& scale = node.scale;
	if (scale.size() == 3)
		this->setScale(glm::vec3(scale[0], scale[1], scale[2]));
}

void Geometry::SceneNode::calculateGlobalModelMatrix() {
	this->calculateLocalModelMatrix();
	globalMatrix_ = this->getLocalModelMatrix();
	
	if (parent) {
		parent->calculateGlobalModelMatrix();
		globalMatrix_ = parent->getGlobalModelMatrix() * globalMatrix_;
	}
}



void Geometry::SceneNode::draw(GeneralApp::Shader& shader) {
	if (isEnabled_) {
		this->calculateGlobalModelMatrix();
		shader.setMat4("model", this->getGlobalModelMatrix());
		mesh_.draw(shader);

		for (auto& child : children) {
			child->draw(shader);
		}
	}
}
