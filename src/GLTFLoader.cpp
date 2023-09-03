#include <iostream>

#include "GLTFLoader.hpp"


namespace GLTF {

GLTF::GLTFLoader* GLTF::GLTFLoader::instancePtr = nullptr;


bool GLTFLoader::load(tinygltf::Model& model, const std::string& filename) {
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename.c_str());
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}

}