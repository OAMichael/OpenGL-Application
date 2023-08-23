#include "../headers/ResourceManager.hpp"
#include "../headers/SceneManager.hpp"
#include "../headers/Model.hpp"


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

	std::vector<int> parentNodesIndices;
	for (int i = 0; i < model_.nodes.size(); ++i) {
		if (nodeIsParentIdx(model_, i))
			parentNodesIndices.push_back(i);
	}

	auto resourceManager = Resources::ResourceManager::getInstance();
	auto sceneManager = SceneResources::SceneManager::getInstance();

	for (int i = 0; i < parentNodesIndices.size(); ++i) {
		auto& gltfNode = model_.nodes[parentNodesIndices[i]];
		auto& newNode = sceneManager->createSceneNode(gltfNode.name);
		
		newNode.init(model_, gltfNode);
		nodes_.push_back(&newNode);
		if(!newNode.children.empty())
			collectChildNodes(nodes_, &newNode);
	}

	for (int i = 1; i < nodes_.size(); ++i) {
		auto& node = nodes_[i];
		if (!node->parent) {
			node->parent = rootNode_;
			rootNode_->children.push_back(node);
		}
	}
	for (auto& node : nodes_) {
		node->calculateGlobalModelMatrix();
	}

#ifdef DEBUG_MODEL
	rootNode_->printNode();
#endif
}

void Model::draw(GeneralApp::Shader& shader) {
	rootNode_->draw(shader);
}


Model::~Model() {
	auto resourceManager = Resources::ResourceManager::getInstance();
	auto sceneManager = SceneResources::SceneManager::getInstance();
	for (auto node : nodes_)
		sceneManager->deleteSceneNode(node->name);
}

}
