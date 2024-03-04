#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Model.hpp"

#include <algorithm>

namespace Geometry {

static inline void collectChildNodes(std::vector<SceneResources::SceneNode*>& presentNodes, SceneResources::SceneNode* currNode) {
	for (auto& child : currNode->children) {
		const std::string newNodeName = child->name;
		if (std::find_if(presentNodes.begin(), presentNodes.end(), 
			[&newNodeName](const SceneResources::SceneNode* node) -> bool { return node->name == newNodeName; }) != presentNodes.end())
			continue;

		presentNodes.push_back(child);
		collectChildNodes(presentNodes, child);
	}
}

static inline bool nodeIsParentIdx(const tinygltf::Model& model, const int idx) {
	for (auto& node : model.nodes)
		if (std::find(node.children.begin(), node.children.end(), idx) != node.children.end())
			return false;

	return true;
}


void Model::init() {
	auto resourceManager = Resources::ResourceManager::getInstance();
	auto sceneManager = SceneResources::SceneManager::getInstance();
	rootNode_ = &sceneManager->createSceneNode(name_);
	rootNode_->setPosition(Position_);
	rootNode_->setRotation(Rotation_);
	rootNode_->setScale(Scale_);

	std::vector<int> parentNodesIndices;
	std::vector<SceneResources::SceneNode*> nodes;

	for (int i = 0; i < model_.nodes.size(); ++i) {
		if (nodeIsParentIdx(model_, i))
			parentNodesIndices.push_back(i);
	}

	for (int i = 0; i < parentNodesIndices.size(); ++i) {
		auto& gltfNode = model_.nodes[parentNodesIndices[i]];
		auto& newNode = sceneManager->createSceneNode(gltfNode.name);
		
		newNode.init(model_, gltfNode);
		nodes.push_back(&newNode);
		if(!newNode.children.empty())
			collectChildNodes(nodes, &newNode);
	}

	for (int i = 0; i < nodes.size(); ++i) {
		auto& node = nodes[i];
		if (!node->parent) {
			node->setParent(rootNode_);
		}
	}
	for (auto& node : nodes) {
		node->calculateGlobalModelMatrix();
	}
}

void Model::draw(Resources::Shader& shader) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	rootNode_->draw(shader);
}

}
