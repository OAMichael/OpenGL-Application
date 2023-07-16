#ifndef MODEL_HPP
#define MODEL_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tinygltf/tiny_gltf.h>

#include "../headers/ISceneObject.hpp"
#include "../headers/SceneNode.hpp"
#include "../headers/Texture.hpp"

namespace Geometry {

class Model final : public ISceneObject {
private:
	tinygltf::Model model_;
	std::string filename_;
	SceneNode* rootNode_;
	std::vector<SceneNode*> nodes_;

public:
	Model() {};
	Model(const std::string& filename);
	Model(const char* filename);

	~Model();

	tinygltf::Model& getModelRef();
	const std::string getFilename();

	void init();
	void draw(GeneralApp::Shader& shader);
};

}
#endif
