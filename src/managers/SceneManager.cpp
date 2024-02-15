#include "SceneManager.hpp"
#include "ResourceManager.hpp"


namespace SceneResources {

static inline SceneHandle createNewSceneHandle() {
    static uint64_t handle = 0;
    ++handle;

    return { handle };
}

SceneManager* SceneManager::instancePtr = nullptr;

SceneNode& SceneManager::createRootNode() {
    if (rootNode_)
        return *rootNode_;

    rootNode_ = &createSceneNode(ROOT_NODE_NAME);
    return *rootNode_;
}

SceneNode& SceneManager::getRootNode() {
    if(rootNode_)
        return *rootNode_;

    return createRootNode();
}

SceneNode& SceneManager::createSceneNode(const std::string& name) {

    SceneNode* newNode = new SceneNode();

#ifdef DEBUG_MODEL
    std::cout << "Creating scene node \'" << name << "\'" << std::endl;
#endif

    newNode->name = name;
    newNode->handle = createNewSceneHandle();
    sceneNodes_[newNode->handle] = newNode;

    return *newNode;
}

SceneLight& SceneManager::createSceneLight(const LightDesc& lightDesc) {
    SceneLight* newLight = new SceneLight();

#ifdef DEBUG_MODEL
    std::cout << "Creating scene light \'" << lightDesc.name << "\'" << std::endl;
#endif

    newLight->name = lightDesc.name;
    newLight->color = lightDesc.color;
    newLight->position = lightDesc.position;
    newLight->direction = lightDesc.direction;
    newLight->cutoff_angle = lightDesc.cutoff_angle;

    newLight->type = lightDesc.type;

    newLight->handle = createNewSceneHandle();
    sceneLights_[newLight->handle] = newLight;

    return *newLight;
}

void SceneManager::deleteSceneNode(const SceneHandle handle) {
    if (auto it = sceneNodes_.find(handle); it != sceneNodes_.end()) {
        delete sceneNodes_[handle];
        sceneNodes_.erase(it);
    }
}

void SceneManager::deleteSceneLight(const SceneHandle handle) {
    if (auto it = sceneLights_.find(handle); it != sceneLights_.end()) {
        delete sceneLights_[handle];
        sceneLights_.erase(it);
    }
}

SceneNode& SceneManager::getSceneNode(const SceneHandle handle) {
    return *sceneNodes_[handle];
}

SceneLight& SceneManager::getSceneLight(const SceneHandle handle) {
    return *sceneLights_[handle];
}

bool SceneManager::hasSceneNode(const std::string& name) {
    auto it = std::find_if(sceneNodes_.begin(), sceneNodes_.end(),
        [&name](const std::pair<SceneHandle, SceneNode*>& pair)
        { return pair.second->name == name; });

    return it != sceneNodes_.end();
}

bool SceneManager::hasSceneLight(const std::string& name) {
    auto it = std::find_if(sceneLights_.begin(), sceneLights_.end(),
        [&name](const std::pair<SceneHandle, SceneLight*>& pair)
        { return pair.second->name == name; });

    return it != sceneLights_.end();
}


void SceneManager::updateLights() {
    std::vector<glm::vec4> pointLightColors;
    std::vector<glm::vec3> pointLightPositions;

    std::vector<glm::vec4> directionalLightColors;
    std::vector<glm::vec3> directionalLightDirections;

    std::vector<glm::vec4> spotLightColors;
    std::vector<glm::vec3> spotLightPositions;
    std::vector<glm::vec3> spotLightDirections;
    std::vector<float>     spotLightCutoffAngles;

    for (auto light : sceneLights_) {
        switch (light.second->type) {
        case SceneLight::LightType::POINT_LIGHT: {
            pointLightColors.push_back(light.second->color);
            pointLightPositions.push_back(light.second->position);

            break;
        }
        case SceneLight::LightType::DIRECTIONAL_LIGHT: {
            directionalLightColors.push_back(light.second->color);
            directionalLightDirections.push_back(light.second->direction);

            break;
        }
        case SceneLight::LightType::SPOT_LIGHT: {
            spotLightColors.push_back(light.second->color);
            spotLightPositions.push_back(light.second->position);
            spotLightDirections.push_back(light.second->direction);
            spotLightCutoffAngles.push_back(light.second->cutoff_angle);

            break;
        }
        }
    }

    const int pointLightNum = pointLightPositions.size();
    const int directionalLightNum = directionalLightDirections.size();
    const int spotLightNum = spotLightPositions.size();

    lightData_.point_light_offset = 0;
    lightData_.directional_light_offset = pointLightNum;
    lightData_.spot_light_offset = pointLightNum + directionalLightNum;
    lightData_.num_of_lights = pointLightNum + directionalLightNum + spotLightNum;

    for (int i = 0; i < pointLightNum; ++i) {
        lightData_.colors[i] = pointLightColors[i];
        lightData_.positions[i] = glm::vec4(pointLightPositions[i], 0.0);
        lightData_.direction_cutoffs[i] = glm::vec4(0.0);
    }

    for (int i = 0; i < directionalLightNum; ++i) {
        lightData_.colors[pointLightNum + i] = directionalLightColors[i];
        lightData_.positions[pointLightNum + i] = glm::vec4(0.0);
        lightData_.direction_cutoffs[pointLightNum + i] = glm::vec4(directionalLightDirections[i], 0.0);
    }

    for (int i = 0; i < spotLightNum; ++i) {
        lightData_.colors[pointLightNum + directionalLightNum + i] = spotLightColors[i];
        lightData_.positions[pointLightNum + directionalLightNum + i] = glm::vec4(spotLightPositions[i], 0.0);
        lightData_.direction_cutoffs[pointLightNum + directionalLightNum + i] = glm::vec4(spotLightDirections[i], spotLightCutoffAngles[i]);
    }
}

const LightData& SceneManager::getLightData() const {
    return lightData_;
}


SceneManager::EnvironmentType SceneManager::getEnvironmentType() {
    return envType_;
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
    texDesc.name = BACKGROUND_2D_TEXTURE_NAME;

    auto& newImage = resourceManager->createImage(textureName);
    texDesc.p_images[0] = &newImage;

    Background2DHandle_ = resourceManager->createTexture(texDesc).handle;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawBackground2D() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    const auto background2DTexture = static_cast<Resources::Texture*>(resourceManager->getResource(Background2DHandle_));
    Resources::Texture& defaultBlackCubemap = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK]);
    Resources::Texture& defaultBlackTexture = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK]);

    glBindVertexArray(VAOBackground2D_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, background2DTexture->GL_id);
    glBindSampler(0, background2DTexture->sampler->GL_id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, defaultBlackCubemap.GL_id);
    glBindSampler(1, defaultBlackCubemap.sampler->GL_id);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, defaultBlackTexture.GL_id);
    glBindSampler(2, defaultBlackTexture.sampler->GL_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
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
    texDesc.name = SKYBOX_TEXTURE_NAME;

    for (unsigned i = 0; i < faces; ++i) {
        auto& newImage = resourceManager->createImage(textureNames[i]);
        texDesc.p_images[i] = &newImage;
    }
    SkyboxHandle_ = resourceManager->createTexture(texDesc).handle;
    resourceManager->generateMipMaps(SkyboxHandle_);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawSkybox() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    const auto skyboxTexture = static_cast<Resources::Texture*>(resourceManager->getResource(SkyboxHandle_));
    Resources::Texture& defaultBlackTexture = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK]);

    glBindVertexArray(VAOSkybox_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultBlackTexture.GL_id);
    glBindSampler(0, defaultBlackTexture.sampler->GL_id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture->GL_id);
    glBindSampler(1, skyboxTexture->sampler->GL_id);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, defaultBlackTexture.GL_id);
    glBindSampler(2, defaultBlackTexture.sampler->GL_id);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createEquirectangular(const std::string& textureName) {
    glGenVertexArrays(1, &VAOEquirect_);
    glGenBuffers(1, &VBOEquirect_);
    glBindVertexArray(VAOEquirect_);

    const std::vector<float>& cubeMapVert = Equirect_.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, VBOEquirect_);
    glBufferData(GL_ARRAY_BUFFER, cubeMapVert.size() * sizeof(float), cubeMapVert.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::TextureDesc texDesc;
    texDesc.faces = 1;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = EQUIRECTANGULAR_TEXTURE_NAME;

    auto& newImage = resourceManager->createImage(textureName);
    texDesc.p_images[0] = &newImage;

    EquirectHandle_ = resourceManager->createTexture(texDesc).handle;
    resourceManager->generateMipMaps(texDesc.name);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawEquirectangular() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    const auto equirectTexture = static_cast<Resources::Texture*>(resourceManager->getResource(EquirectHandle_));
    Resources::Texture& defaultBlackCubemap = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK]);
    Resources::Texture& defaultBlackTexture = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK]);

    glBindVertexArray(VAOEquirect_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, defaultBlackTexture.GL_id);
    glBindSampler(0, defaultBlackTexture.sampler->GL_id);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, defaultBlackCubemap.GL_id);
    glBindSampler(1, defaultBlackCubemap.sampler->GL_id);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, equirectTexture->GL_id);
    glBindSampler(2, equirectTexture->sampler->GL_id);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames) {
    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = ENVIRONMENT_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = "../shaders/DefaultEnv.vert";
    shaderDesc.fragFilename = "../shaders/DefaultEnv.frag";

    auto& envShader = resourceManager->createShader(shaderDesc);

    envShader.use();
    envShader.setUint("environmentType", (uint32_t)envType);
    envShader.setInt("uSamplerBackground2D", 0);
    envShader.setInt("uSamplerSkybox", 1);
    envShader.setInt("uSamplerEquirect", 2);

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

    case EnvironmentType::EQUIRECTANGULAR:
        if (textureNames.size() != 1)
            return;

        createEquirectangular(textureNames[0]);
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
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& envShader = resourceManager->getShader(ENVIRONMENT_SHADER_NAME);
    envShader.use();
    switch (envType_) {
    case EnvironmentType::BACKGROUND_IMAGE_2D:
        drawBackground2D();
        break;

    case EnvironmentType::SKYBOX:
        drawSkybox();
        break;

    case EnvironmentType::EQUIRECTANGULAR:
        drawEquirectangular();
        break;

    default:;
    }
}


const Resources::ResourceHandle SceneManager::getBackground2DHandle() const {
    return Background2DHandle_;
}

const Resources::ResourceHandle SceneManager::getSkyboxHandle() const {
    return SkyboxHandle_;
}

const Resources::ResourceHandle SceneManager::getEquirectangularHandle() const {
    return EquirectHandle_;
}


void SceneManager::createFullscreenQuad() {
    auto* resourceManager = Resources::ResourceManager::getInstance();

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAOFullscreenQuad_);
    glGenBuffers(1, &VBOFullscreenQuad_);
    glBindVertexArray(VAOFullscreenQuad_);
    glBindBuffer(GL_ARRAY_BUFFER, VBOFullscreenQuad_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);


    Resources::ShaderDesc shdrDesc;
    shdrDesc.name = FULLSCREEN_QUAD_SHADER_NAME;
    shdrDesc.uri = "";
    shdrDesc.vertFilename = "../shaders/FullscreenQuad.vert";
    shdrDesc.fragFilename = "../shaders/FullscreenQuad.frag";
    auto& shdr = resourceManager->createShader(shdrDesc);

    shdr.use();
    shdr.setInt("uScreenTexture", 0);
    shdr.setBool("uEnableBlur", 0);
}

void SceneManager::drawFullscreenQuad(const std::string& textureName) {
    // Assume that proper framebuffer already bound

    auto* resourceManager = Resources::ResourceManager::getInstance();

    int depthTestEnabled = 0;
    glGetIntegerv(GL_DEPTH_TEST, &depthTestEnabled);

    if (depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }

    resourceManager->getShader(FULLSCREEN_QUAD_SHADER_NAME).use();
    glBindVertexArray(VAOFullscreenQuad_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resourceManager->getTexture(textureName).GL_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

void SceneManager::setEnableBlur(bool enabled) {
    auto* resourceManager = Resources::ResourceManager::getInstance();
    auto& fullscreenQuadShader = resourceManager->getShader(FULLSCREEN_QUAD_SHADER_NAME);

    fullscreenQuadShader.setBool("uEnableBlur", enabled);
}


SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
    for (auto& node : sceneNodes_) {
        delete node.second;
        sceneNodes_.erase(node.first);
    }

    for (auto& light : sceneLights_) {
        delete light.second;
        sceneLights_.erase(light.first);
    }

    rootNode_ = nullptr;
}
}
