#ifndef JSON_IMPORTER_HPP
#define JSON_IMPORTER_HPP

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace JsonUtil {

    class JSONImpoter final {
    private:

        static JSONImpoter* instancePtr;

        JSONImpoter() {};

    public:

        struct ModelImportInfo {
            std::string name;
            std::string filename;
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;
        };

        JSONImpoter(const JSONImpoter& obj) = delete;

        static JSONImpoter* getInstance() {
            if (!instancePtr)
                instancePtr = new JSONImpoter();

            return instancePtr;
        }

        void loadModelsInfo(const nlohmann::json& cfg, std::vector<ModelImportInfo>& modelInfos);
    };
}

#endif