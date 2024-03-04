#ifndef MODEL_HPP
#define MODEL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tinygltf/tiny_gltf.h>

#include "ISceneObject.hpp"
#include "SceneNode.hpp"
#include "Texture.hpp"

namespace Geometry {

class Model final : public SceneResources::ISceneObject {
private:
	tinygltf::Model model_;
	std::string filename_;
	SceneResources::SceneNode* rootNode_ = nullptr;

	std::string name_;

public:
	Model(const std::string& name, const std::string& filename) : name_{ name }, filename_{ filename } {};
	Model() {};
	~Model() {};

	inline tinygltf::Model& getModelRef() { return model_; }
	inline const std::string& getFilename() const { return filename_; }
	inline void setFilename(const std::string& filename) { filename_ = filename; }
	inline const std::string& getName() const { return name_; }
	inline void setName(const std::string& name) { name_ = name; }
	inline SceneResources::SceneNode* getModelRootNode() { return rootNode_; }

	void init();
	void draw(Resources::Shader& shader);
};

}
#endif
