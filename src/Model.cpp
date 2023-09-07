#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Model.hpp"


namespace Geometry {

static inline void collectChildNodes(std::vector<Geometry::SceneNode*>& presentNodes, Geometry::SceneNode* currNode) {
	for (auto& child : currNode->children) {
		const std::string newNodeName = child->name;
		if (std::find_if(presentNodes.begin(), presentNodes.end(), [&newNodeName](const SceneNode* node) -> bool { return node->name == newNodeName; }) != presentNodes.end())
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


Model::Model(const std::string& name, const std::string& filename) {
	filename_ = filename;
	name_ = name;
}

tinygltf::Model& Model::getModelRef() {
	return model_;
}

const std::string& Model::getFilename() const {
	return filename_;
}

void Model::setFilename(const std::string& filename) {
	filename_ = filename;
}


const std::string& Model::getName() const {
	return name_;
}

void Model::setName(const std::string& name) {
	name_ = name;
}

SceneNode* Model::getModelRootNode() {
	return rootNode_;
}


void Model::init() {
	auto resourceManager = Resources::ResourceManager::getInstance();
	auto sceneManager = SceneResources::SceneManager::getInstance();
	rootNode_ = &sceneManager->createSceneNode(name_);
	rootNode_->setPosition(Position_);
	rootNode_->setRotation(Rotation_);
	rootNode_->setScale(Scale_);

	std::vector<int> parentNodesIndices;
	std::vector<SceneNode*> nodes;

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

void Model::draw(GeneralApp::Shader& shader) {
	rootNode_->draw(shader);
}


Model::~Model() {

}

}
