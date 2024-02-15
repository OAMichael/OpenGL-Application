#include <iostream>

#include "GLTFLoader.hpp"


namespace GLTF {

GLTFLoader* GLTFLoader::instancePtr = nullptr;


bool GLTFLoader::load(Geometry::Model& model, const std::string& filename) {
    std::string err;
    std::string warn;

    bool res = loader_.LoadASCIIFromFile(&model.getModelRef(), &err, &warn, filename.c_str());
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else {
        loadedModels[model.getName()] = &model;
        std::cout << "Loaded glTF: " << filename << std::endl;
    }
    return res;
}

}