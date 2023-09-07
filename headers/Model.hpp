#ifndef MODEL_HPP
#define MODEL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tinygltf/tiny_gltf.h>

#include "ISceneObject.hpp"
#include "SceneNode.hpp"
#include "Texture.hpp"

namespace Geometry {

class Model final : public ISceneObject {
private:
	tinygltf::Model model_;
	std::string filename_;
	SceneNode* rootNode_ = nullptr;

	std::string name_;

public:
	Model() {};
	Model(const std::string& name, const std::string& filename);

	~Model();

	tinygltf::Model& getModelRef();
	const std::string& getFilename() const;
	void setFilename(const std::string& filename);
	const std::string& getName() const;
	void setName(const std::string& name);
	SceneNode* getModelRootNode();

	void init();
	void draw(GeneralApp::Shader& shader);
};

}
#endif
