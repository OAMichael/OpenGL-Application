#ifndef GLTFLOADER_HPP
#define GLTFLOADER_HPP

#include <tinygltf/tiny_gltf.h>

namespace GLTF {

class GLTFLoader final {
private:
    tinygltf::TinyGLTF loader;

    static GLTFLoader* instancePtr;

    GLTFLoader() {};

public:

    GLTFLoader(const GLTFLoader& obj) = delete;

    static GLTFLoader* getInstance() {
        if (!instancePtr)
            instancePtr = new GLTFLoader();

        return instancePtr;
    }

    bool load(tinygltf::Model& model, const std::string& filename);
};

}

#endif