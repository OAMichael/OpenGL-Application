#include "../headers/Model.hpp"

namespace Geometry {


Model::Model(const std::string& filename) {
	filename_ = filename;

	Position_ = glm::vec3(0.0f);
}

Model::Model(const char* filename) {
	if (!filename) {
		std::cout << "Model filename cannot be NULL !!!" << std::endl;
	}
	else {
		filename_ = std::string(filename);
		Position_ = glm::vec3(0.0f);
	}
}

tinygltf::Model& Model::getModelRef() {
	return model_;
}

const std::string Model::getFilename() {
	return filename_;
}

void Model::init() {
	rootNode_ = new SceneNode();
	nodes_.push_back(rootNode_);
	rootNode_->name = "RootNode";
	rootNode_->setScale(glm::vec3(10.0f));

	for (int i = 0; i < model_.nodes.size(); ++i) {
		SceneNode* newNode = new SceneNode();

		newNode->init(model_, model_.nodes[i]);
		if (!newNode->parent) {
			newNode->parent = rootNode_;
			rootNode_->children.push_back(newNode);
		}

		nodes_.push_back(newNode);
	}
}

void Model::draw(GeneralApp::Shader& shader) {
	rootNode_->draw(shader);
}


Model::~Model() {
	for (int i = 0; i < nodes_.size(); ++i)
		delete nodes_[i];
}


}
