#ifndef MESH_HPP
#define MESH_HPP

#include "Shader.hpp"
#include "Texture.hpp"

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
    void draw(Resources::Shader& shader);

    std::string name;

private:
    const tinygltf::Model* modelPtr_ = nullptr;
    const tinygltf::Mesh* meshPtr_ = nullptr;

    std::unordered_map<int, unsigned int> VBOs_;
    std::unordered_map<int, unsigned int> VAOs_;

    std::unordered_map<int, Resources::ResourceHandle> primitiveMaterial_; // primitive index, material
};

}
#endif
