#include <iostream>

#include "JSONImporter.hpp"


namespace JsonUtil {

JSONImpoter* JSONImpoter::instancePtr = nullptr;

void JSONImpoter::loadModelsInfo(const nlohmann::json& cfg, std::vector<ModelImportInfo>& modelInfos) {
    const auto& models = cfg.find("models");
    if (models != cfg.end() && models->is_array()) {
        for (auto modelIt = models->begin(); modelIt != models->end(); ++modelIt) {
            std::string name = "";
            if (auto modelName = modelIt->find("name"); modelName != modelIt->end() && modelName->is_string())
                name = *modelName;

            std::string filename = "";
            if (auto modelFilename = modelIt->find("filename"); modelFilename != modelIt->end() && modelFilename->is_string())
                filename = *modelFilename;

            glm::vec3 position = glm::vec3(0.0);
            if (auto modelPosition = modelIt->find("position"); modelPosition != modelIt->end() && modelPosition->is_array()) {
                unsigned i = 0;
                for (auto posIt = modelPosition->begin(); posIt != modelPosition->end(); ++posIt, ++i) {
                    if (!posIt->is_number())
                        continue;

                    position[i] = *posIt;
                }
            }

            glm::quat rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
            if (auto modelRotation = modelIt->find("rotation"); modelRotation != modelIt->end() && modelRotation->is_array()) {
                unsigned i = 0;
                for (auto rotIt = modelRotation->begin(); rotIt != modelRotation->end(); ++rotIt, ++i) {
                    if (!rotIt->is_number())
                        continue;

                    rotation[(i + 3) % 4] = *rotIt;
                }
            }

            glm::vec3 scale = glm::vec3(1.0);
            if (auto modelScale = modelIt->find("scale"); modelScale != modelIt->end() && modelScale->is_array()) {
                unsigned i = 0;
                for (auto sclIt = modelScale->begin(); sclIt != modelScale->end(); ++sclIt, ++i) {
                    if (!sclIt->is_number())
                        continue;

                    scale[i] = *sclIt;
                }
            }

            modelInfos.push_back({ name, filename, position, rotation, scale });
        }
    }
}

}