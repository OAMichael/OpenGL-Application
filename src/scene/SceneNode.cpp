#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "SceneNode.hpp"

void SceneResources::SceneNode::init(const tinygltf::Model& model, const tinygltf::Node& node) {
	if (parent)
		return;

	if (node.mesh > -1 && node.mesh < model.meshes.size()) {
		mesh_ = Geometry::Mesh(model, model.meshes[node.mesh]);
		mesh_.init();
	}

	name = node.name;

	auto resourceManager = Resources::ResourceManager::getInstance();
	auto sceneManager = SceneResources::SceneManager::getInstance();

	for (auto& child : node.children) {
		auto& childNode = sceneManager->createSceneNode(model.nodes[child].name);
		childNode.init(model, model.nodes[child]);
		childNode.parent = this;

		children.push_back(&childNode);
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

void SceneResources::SceneNode::calculateGlobalModelMatrix() {
	this->calculateLocalModelMatrix();
	globalMatrix_ = this->getLocalModelMatrix();
	
	if (parent) {
		parent->calculateGlobalModelMatrix();
		globalMatrix_ = parent->getGlobalModelMatrix() * globalMatrix_;
	}
}



void SceneResources::SceneNode::draw(GeneralApp::Shader& shader) {
	if (isEnabled_) {
		if (!isRoot()) {
			auto resourceManager = Resources::ResourceManager::getInstance();
			glm::mat4 transform = this->getGlobalModelMatrix();
			resourceManager->updateBuffer("Matrices", (const unsigned char*)&transform, sizeof(transform), 2 * sizeof(glm::mat4));
			mesh_.draw(shader);
		}
		for (auto& child : children) {
			child->draw(shader);
		}
	}
}


void SceneResources::SceneNode::setParent(SceneNode* par) {
	parent = par;
	if (par && std::find(par->children.begin(), par->children.end(), this) == par->children.end())
		par->children.push_back(this);
}


void SceneResources::SceneNode::printNode(const int level) {
	static int lastLevel = 0;
	lastLevel = level;
	if (level > 1)
		for (unsigned i = 0; i < level - 1; ++i)
			printf("|   ");

	if (level > 0)
		printf("|---");

	printf("%s\n", name.length() > 0 ? name.c_str() : "<unnamed>");
	for (auto& child : children)
		child->printNode(level + 1);

	if (!isRoot() && level < lastLevel) {
		for (unsigned i = 0; i < level; ++i)
			printf("|   ");
		
		printf("\n");
	}
}
