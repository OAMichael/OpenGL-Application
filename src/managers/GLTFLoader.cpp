#include <iostream>

#include "GLTFLoader.hpp"
#include "FileManager.hpp"
#include "Logger.hpp"

namespace GLTF {

GLTFLoader* GLTFLoader::instancePtr = nullptr;


bool GLTFLoader::load(Geometry::Model& model, const std::string& filename) {
    auto fileManager = FileSystem::FileManager::getInstance();
    std::string absPath = fileManager->getAbsolutePath(filename);

    std::string err;
    std::string warn;

    bool res = loader_.LoadASCIIFromFile(&model.getModelRef(), &err, &warn, absPath.c_str());
    if (!warn.empty()) {
        LOG_W("%s", warn.c_str());
    }

    if (!err.empty()) {
        LOG_E("%s", err.c_str());
    }

    if (!res) {
        LOG_E("Failed to load glTF: %s", filename.c_str());
    }
    else {
        loadedModels[model.getName()] = &model;
        LOG_I("Loaded glTF: %s", filename.c_str());
    }
    return res;
}

}