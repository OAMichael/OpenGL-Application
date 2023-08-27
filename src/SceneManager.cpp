#include "../headers/SceneManager.hpp"
#include "../headers/ResourceManager.hpp"


namespace SceneResources {

SceneManager* SceneManager::instancePtr = nullptr;

Geometry::SceneNode& SceneManager::createSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end())
        return *sceneNodes_[name];

    Geometry::SceneNode* newNode = new Geometry::SceneNode();

#ifdef DEBUG_MODEL
    std::cout << "Creating scene node \'" << name << "\'" << std::endl;
#endif

    newNode->name = name;
    sceneNodes_[newNode->name] = newNode;

    return *newNode;
}

void SceneManager::deleteSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end()) {
        delete sceneNodes_[name];
        sceneNodes_.erase(it);
    }
}

Geometry::SceneNode& SceneManager::getSceneNode(const std::string& name) {
    if (auto it = sceneNodes_.find(name); it != sceneNodes_.end())
        return *sceneNodes_[name];

    auto emptyNode = new Geometry::SceneNode();
    sceneNodes_[name] = emptyNode;
    return *emptyNode;
}

bool SceneManager::hasSceneNode(const std::string& name) {
    auto it = sceneNodes_.find(name);
    return it != sceneNodes_.end();
}


void SceneManager::createSkybox(const std::vector<std::string>& textureNames) {
    glGenVertexArrays(1, &VAOSkybox_);
    glGenBuffers(1, &VBOSkybox_);
    glBindVertexArray(VAOSkybox_);

    const std::vector<float>& vertsSkybox = Skybox_.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, VBOSkybox_);
    glBufferData(GL_ARRAY_BUFFER, vertsSkybox.size() * sizeof(float), vertsSkybox.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    auto resourceManager = Resources::ResourceManager::getInstance();

    unsigned faces = 6;
    Resources::TextureDesc texDesc;
    texDesc.faces = 6;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = "SKYBOX_TEXTURE";

    for (unsigned i = 0; i < faces; ++i) {
        auto& newImage = resourceManager->createImage(textureNames[i]);
        texDesc.p_images[i] = &newImage;
    }
    resourceManager->createTexture(texDesc);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawSkybox() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    const auto& skyboxTexture = resourceManager->getTexture("SKYBOX_TEXTURE");

    glBindVertexArray(VAOSkybox_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.GL_id);
    glBindSampler(0, skyboxTexture.sampler->GL_id);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createBackground2D(const std::string& textureName) {
    glGenVertexArrays(1, &VAOBackground2D_);
    glGenBuffers(1, &VBOBackground2D_);
    glBindVertexArray(VAOBackground2D_);

    const std::vector<float>& vertsBackground2D = {
        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,     1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBOBackground2D_);
    glBufferData(GL_ARRAY_BUFFER, vertsBackground2D.size() * sizeof(float), vertsBackground2D.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::TextureDesc texDesc;
    texDesc.faces = 1;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = "BACKGROUND_2D_TEXTURE";

    auto& newImage = resourceManager->createImage(textureName);
    texDesc.p_images[0] = &newImage;

    resourceManager->createTexture(texDesc);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawBackground2D() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    const auto& background2DTexture = resourceManager->getTexture("BACKGROUND_2D_TEXTURE");

    glBindVertexArray(VAOBackground2D_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, background2DTexture.GL_id);
    glBindSampler(0, background2DTexture.sampler->GL_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void SceneManager::createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames) {

    defaultEnvShader_ = GeneralApp::Shader("../shaders/DefaultEnv.vert", "../shaders/DefaultEnv.frag");
    defaultEnvShader_.use();
    defaultEnvShader_.setUint("environmentType", (uint32_t)envType);

    envType_ = envType;
    switch (envType) {
    case EnvironmentType::BACKGROUND_IMAGE_2D:
        if (textureNames.size() != 1)
            return;

        createBackground2D(textureNames[0]);
        break;

    case EnvironmentType::SKYBOX:
        if (textureNames.size() != 6)
            return;

        createSkybox(textureNames);
        break;

    default:
        std::cout << "Unknown environment type" << std::endl;
    }
}

void SceneManager::createEnvironment(const EnvironmentType envType, const std::string& textureName) {
    std::vector<std::string> tmpVec;
    tmpVec.push_back(textureName);
    createEnvironment(envType, tmpVec);
}

void SceneManager::drawEnvironment() {
    defaultEnvShader_.use();
    switch (envType_) {
    case EnvironmentType::BACKGROUND_IMAGE_2D:
        drawBackground2D();
        break;

    case EnvironmentType::SKYBOX:
        drawSkybox();
        break;

    default:;
    }
}


SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
    for (auto& node : sceneNodes_) {
        delete node.second;
        sceneNodes_.erase(node.first);
    }
}
}
