#ifndef GLTFLOADER_HPP
#define GLTFLOADER_HPP

#include <tinygltf/tiny_gltf.h>

#include <Model.hpp>

namespace GLTF {

class GLTFLoader final {
private:
    tinygltf::TinyGLTF loader_;

    static GLTFLoader* instancePtr;

    GLTFLoader() {};

public:

    std::unordered_map<std::string, Geometry::Model*> loadedModels;

    GLTFLoader(const GLTFLoader& obj) = delete;

    static GLTFLoader* getInstance() {
        if (!instancePtr)
            instancePtr = new GLTFLoader();

        return instancePtr;
    }

    bool load(Geometry::Model& model, const std::string& filename);
};

}

#endif