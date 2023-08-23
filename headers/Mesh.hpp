#ifndef MESH_HPP
#define MESH_HPP

#include "../headers/Shader.hpp"
#include "../headers/Texture.hpp"

#include <vector>
#include <unordered_map>

#include <tinygltf/tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


namespace Geometry {


class Mesh {
public:

    Mesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) : modelPtr_{ &model }, meshPtr_{ &mesh } {}
    Mesh() {}
    ~Mesh() {}

    void init();
    void draw(GeneralApp::Shader& shader);

    std::string name;

private:
    const tinygltf::Model* modelPtr_ = nullptr;
    const tinygltf::Mesh* meshPtr_ = nullptr;

    unsigned int VAO_;
    std::unordered_map<int, unsigned int> VBOs_;

    std::unordered_map<int, std::string> primitiveMaterial_;
};

}
#endif
