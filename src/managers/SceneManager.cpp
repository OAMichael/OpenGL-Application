#include "SceneManager.hpp"
#include "ResourceManager.hpp"
#include "Logger.hpp"

#include <algorithm>

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

    LOG_I("Creating scene node \'%s\'", name.c_str());

    newNode->name = name;
    newNode->handle = createNewSceneHandle();
    sceneNodes_[newNode->handle] = newNode;

    return *newNode;
}

SceneLight& SceneManager::createSceneLight(const LightDesc& lightDesc) {
    SceneLight* newLight = new SceneLight();

    LOG_I("Creating scene light \'%s\'", lightDesc.name.c_str());

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
        LOG_I("Deleting scene node \'%s\'", sceneNodes_[handle]->name.c_str());
        delete sceneNodes_[handle];
        sceneNodes_.erase(it);
    }
}

void SceneManager::deleteSceneLight(const SceneHandle handle) {
    if (auto it = sceneLights_.find(handle); it != sceneLights_.end()) {
        LOG_I("Deleting scene light \'%s\'", sceneLights_[handle]->name.c_str());
        delete sceneLights_[handle];
        sceneLights_.erase(it);
    }
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


void SceneManager::createBackground2D(const std::string& textureName, bool isHdr) {
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
    if (isHdr) {
        texDesc.format = GL_RGBA32F;
    }

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
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindSampler(0, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createSkybox(const std::vector<std::string>& textureNames, bool isHdr) {
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

    if (isHdr) {
        texDesc.format = GL_RGBA32F;
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

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindSampler(0, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createEquirectangular(const std::string& textureName, bool isHdr) {
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

    auto& newImage = resourceManager->createImage(textureName, isHdr);
    texDesc.p_images[0] = &newImage;
    if (isHdr) {
        texDesc.format = GL_RGBA32F;
    }

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
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindSampler(0, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}


void SceneManager::createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames, bool isHdr) {
    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = ENVIRONMENT_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = "../shaders/DefaultEnv.vert";
    shaderDesc.fragFilename = "../shaders/DefaultEnv.frag";

    auto& envShader = resourceManager->createShader(shaderDesc);

    envShader.use();
    envShader.setInt("uSamplerBackground2D", 0);
    envShader.setInt("uSamplerSkybox", 1);
    envShader.setInt("uSamplerEquirect", 2);

    if (envType < 0 || envType >= EnvironmentType::COUNT) {
        LOG_W("Unknown environment type");
        return;
    }

    setEnvironmentType(envType);
    envHdr_[envType] = isHdr;

    switch (envType) {
    case EnvironmentType::BACKGROUND_IMAGE_2D:
        if (textureNames.size() != 1)
            return;

        createBackground2D(textureNames[0], isHdr);
        break;

    case EnvironmentType::SKYBOX:
        if (textureNames.size() != 6)
            return;

        createSkybox(textureNames, isHdr);
        break;

    case EnvironmentType::EQUIRECTANGULAR:
        if (textureNames.size() != 1)
            return;

        createEquirectangular(textureNames[0], isHdr);
        break;

    default:
        LOG_W("Unknown environment type");
    }
}

void SceneManager::createEnvironment(const EnvironmentType envType, const std::string& textureName, bool isHdr) {
    std::vector<std::string> tmpVec;
    tmpVec.push_back(textureName);
    createEnvironment(envType, tmpVec, isHdr);
}

void SceneManager::drawEnvironment() {
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& envShader = resourceManager->getShader(ENVIRONMENT_SHADER_NAME);
    envShader.use();
    envShader.setUint("uEnvironmentType", (uint32_t)envType_);
    envShader.setUint("uIsHdr", envHdr_[envType_]);

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

    default:
        LOG_W("Unknown environment type");
    }
}


void SceneManager::createPostProcess(const PostProcessInfo& ppi) {
    postProcessInfo_ = ppi;

    createFullscreenQuad();

    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = GAUSSIAN_BLUR_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = "../shaders/FullscreenQuad.vert";
    shaderDesc.fragFilename = "../shaders/PostProcess/GaussianBlur.frag";

    auto& blurShader = resourceManager->createShader(shaderDesc);
    blurShader.use();
    blurShader.setInt("uScreenTexture", 0);


    shaderDesc.name = BLOOM_SHADER_NAME;
    shaderDesc.vertFilename = "../shaders/FullscreenQuad.vert";
    shaderDesc.fragFilename = "../shaders/PostProcess/Bloom.frag";

    auto& bloomShader = resourceManager->createShader(shaderDesc);
    bloomShader.use();
    bloomShader.setInt("uScreenTexture", 0);


    shaderDesc.name = BLOOM_FINAL_SHADER_NAME;
    shaderDesc.vertFilename = "../shaders/FullscreenQuad.vert";
    shaderDesc.fragFilename = "../shaders/PostProcess/BloomFinal.frag";

    auto& bloomFinalShader = resourceManager->createShader(shaderDesc);
    bloomFinalShader.use();
    bloomFinalShader.setInt("uSceneColor", 0);
    bloomFinalShader.setInt("uBloomBlur", 1);


    Resources::ImageDesc fbImageDesc;
    fbImageDesc.uri = "";
    fbImageDesc.width = postProcessInfo_.windowWidth;
    fbImageDesc.height = postProcessInfo_.windowHeight;
    fbImageDesc.components = 4;
    fbImageDesc.bits = 8 * sizeof(float);
    fbImageDesc.format = GL_RGBA;
    fbImageDesc.p_data = nullptr;

    Resources::TextureDesc fbTextureDesc;
    fbTextureDesc.uri = "";

    Resources::FramebufferDesc fbDesc;
    fbDesc.uri = "";
    fbDesc.dependency = Resources::defaultFramebufferName;


    /* Create Gaussian Blur X */
    {
        fbImageDesc.name = "GAUSSIAN_BLUR_X_IMAGE";
        Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_X_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTexture = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_X_TEXTURE_DEPTH";
        fbTextureDesc.format = GL_DEPTH_COMPONENT24;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureDepth = resourceManager->createTexture(fbTextureDesc);

        fbDesc.name = "GAUSSIAN_BLUR_X_FRAMEBUFFER";
        fbDesc.colorAttachmentsCount = 1;
        fbDesc.colorAttachments[0] = &fbTexture;
        fbDesc.depthAttachment = &fbTextureDepth;
        Resources::Framebuffer& fb = resourceManager->createFramebuffer(fbDesc);

        blurXFramebufferHandle_ = fb.handle;
    }


    /* Create Gaussian Blur Y */
    {
        fbImageDesc.name = "GAUSSIAN_BLUR_Y_IMAGE";
        Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_Y_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTexture = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_Y_TEXTURE_DEPTH";
        fbTextureDesc.format = GL_DEPTH_COMPONENT24;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureDepth = resourceManager->createTexture(fbTextureDesc);

        fbDesc.name = "GAUSSIAN_BLUR_Y_FRAMEBUFFER";
        fbDesc.colorAttachmentsCount = 1;
        fbDesc.colorAttachments[0] = &fbTexture;
        fbDesc.depthAttachment = &fbTextureDepth;
        Resources::Framebuffer& fb = resourceManager->createFramebuffer(fbDesc);

        blurYFramebufferHandle_ = fb.handle;
    }


    /* Create Bloom Buffers */
    {
        fbImageDesc.name = "BLOOM_COLOR_IMAGE";
        Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

        fbTextureDesc.name = "BLOOM_COLOR_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureColor = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "BLOOM_BRIGHTNESS_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureBrightness = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "BLOOM_COLOR_TEXTURE_DEPTH";
        fbTextureDesc.format = GL_DEPTH_COMPONENT24;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureDepth = resourceManager->createTexture(fbTextureDesc);

        fbDesc.name = "BLOOM_COLOR_FRAMEBUFFER";
        fbDesc.colorAttachmentsCount = 2;
        fbDesc.colorAttachments[0] = &fbTextureColor;
        fbDesc.colorAttachments[1] = &fbTextureBrightness;
        fbDesc.depthAttachment = &fbTextureDepth;
        Resources::Framebuffer& fb = resourceManager->createFramebuffer(fbDesc);

        bloomFramebufferHandle_ = fb.handle;
    }


    /* Create Final Bloom */
    {
        fbImageDesc.name = "BLOOM_FINAL_IMAGE";
        Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

        fbTextureDesc.name = "BLOOM_FINAL_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTexture = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "BLOOM_FINAL_TEXTURE_DEPTH";
        fbTextureDesc.format = GL_DEPTH_COMPONENT24;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTextureDepth = resourceManager->createTexture(fbTextureDesc);

        fbDesc.name = "BLOOM_FINAL_FRAMEBUFFER";
        fbDesc.colorAttachmentsCount = 1;
        fbDesc.colorAttachments[0] = &fbTexture;
        fbDesc.depthAttachment = &fbTextureDepth;
        Resources::Framebuffer& fb = resourceManager->createFramebuffer(fbDesc);

        bloomFinalFramebufferHandle_ = fb.handle;
    }
}

void SceneManager::performPostProcess(const Resources::ResourceHandle inputTextureHandle) {
    postProcessTextureHandle_ = inputTextureHandle;

    auto resourceManager = Resources::ResourceManager::getInstance();

    if (postProcessInfo_.enableBloom) {
        auto& bloomShader = resourceManager->getShader(BLOOM_SHADER_NAME);
        auto& blurShader = resourceManager->getShader(GAUSSIAN_BLUR_SHADER_NAME);
        auto& bloomFinalShader = resourceManager->getShader(BLOOM_FINAL_SHADER_NAME);

        auto* bloomFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(bloomFramebufferHandle_));
        auto* blurXFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(blurXFramebufferHandle_));
        auto* blurYFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(blurYFramebufferHandle_));
        auto* bloomFinalFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(bloomFinalFramebufferHandle_));

        auto* inputTexture = static_cast<Resources::Texture*>(resourceManager->getResource(postProcessTextureHandle_));
        auto* bloomColorTexture = bloomFramebuffer->colorAttachments[0];
        auto* bloomBrightnessTexture = bloomFramebuffer->colorAttachments[1];
        auto* blurXTexture = blurXFramebuffer->colorAttachments[0];
        auto* blurYTexture = blurYFramebuffer->colorAttachments[0];
        auto* bloomFinalTexture = bloomFinalFramebuffer->colorAttachments[0];


        resourceManager->bindFramebuffer(bloomFramebuffer->name);
        drawFullscreenQuad(postProcessTextureHandle_, &bloomShader);

        blurShader.use();
        resourceManager->bindFramebuffer(blurXFramebuffer->name);
        blurShader.setBool("uHorizontal", true);
        drawFullscreenQuad(bloomBrightnessTexture->handle, &blurShader);

        resourceManager->bindFramebuffer(blurYFramebuffer->name);
        blurShader.setBool("uHorizontal", false);
        drawFullscreenQuad(blurXTexture->handle, &blurShader);

        resourceManager->bindFramebuffer(bloomFinalFramebuffer->name);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blurYTexture->GL_id);
        drawFullscreenQuad(bloomColorTexture->handle, &bloomFinalShader);        // Setups only texture0


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        postProcessTextureHandle_ = bloomFinalTexture->handle;
    }

    if (postProcessInfo_.enableBlur) {
        auto& blurShader = resourceManager->getShader(GAUSSIAN_BLUR_SHADER_NAME);

        auto* blurXFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(blurXFramebufferHandle_));
        auto* blurYFramebuffer = static_cast<Resources::Framebuffer*>(resourceManager->getResource(blurYFramebufferHandle_));

        auto* inputTexture = static_cast<Resources::Texture*>(resourceManager->getResource(postProcessTextureHandle_));
        auto* blurXTexture = blurXFramebuffer->colorAttachments[0];
        auto* blurYTexture = blurYFramebuffer->colorAttachments[0];


        blurShader.use();
        resourceManager->bindFramebuffer(blurXFramebuffer->name);
        blurShader.setBool("uHorizontal", true);
        drawFullscreenQuad(postProcessTextureHandle_, &blurShader);

        resourceManager->bindFramebuffer(blurYFramebuffer->name);
        blurShader.setBool("uHorizontal", false);
        drawFullscreenQuad(blurXTexture->handle, &blurShader);

        postProcessTextureHandle_ = blurYTexture->handle;
    }
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
}

void SceneManager::drawFullscreenQuad(const Resources::ResourceHandle inputTextureHandle, Resources::Shader* shader) {
    // Assume that proper framebuffer already bound
    auto* resourceManager = Resources::ResourceManager::getInstance();
    Resources::RenderResource* resource = resourceManager->getResource(inputTextureHandle);
    if (resource->type != Resources::RenderResource::ResourceType::TEXTURE) {
        LOG_E("Improper resource handle");
        return;
    }

    Resources::Texture* texture = static_cast<Resources::Texture*>(resource);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (shader) {
        shader->use();
    }
    else {
        resourceManager->getShader(FULLSCREEN_QUAD_SHADER_NAME).use();
    }

    glBindVertexArray(VAOFullscreenQuad_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->GL_id);
    glBindSampler(0, texture->sampler->GL_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);
    glBindVertexArray(0);
}

void SceneManager::drawToDefaultFramebuffer(const Resources::ResourceHandle inputTextureHandle) {
    auto* resourceManager = Resources::ResourceManager::getInstance();
    if (!resourceManager->hasShader(FULLSCREEN_QUAD_SHADER_NAME)) {
        createFullscreenQuad();
    }

    resourceManager->bindFramebuffer(Resources::defaultFramebufferName);
    drawFullscreenQuad(inputTextureHandle);
}


void SceneManager::cleanUp() {
    while (!sceneNodes_.empty()) {
        auto it = sceneNodes_.begin();
        deleteSceneNode(it->first);
    }

    while (!sceneLights_.empty()) {
        auto it = sceneLights_.begin();
        deleteSceneLight(it->first);
    }

    rootNode_ = nullptr;
}

bool SceneManager::initializeFreeType(const std::string& fontFilename, const unsigned fontHeight) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LOG_E("FreeType: could not init FreeType Library");
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontFilename.c_str(), 0, &face)) {
        LOG_E("FreeType: failed to load font");
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, fontHeight);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    auto resourceManager = Resources::ResourceManager::getInstance();

    for (unsigned char c = 32; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_E("FreeType: failed to load glyph for \'%c\'", c);
            continue;
        }

        Resources::ImageDesc charImageDesc;
        charImageDesc.name = "image_" + std::string(1, c);
        charImageDesc.format = GL_RED;
        charImageDesc.width = face->glyph->bitmap.width;
        charImageDesc.height = face->glyph->bitmap.rows;
        charImageDesc.bits = 8;
        charImageDesc.components = 1;
        charImageDesc.p_data = face->glyph->bitmap.buffer;

        auto& charImage = resourceManager->createImage(charImageDesc);


        Resources::SamplerDesc charSamplerDesc;
        charSamplerDesc.name = "sampler_" + std::string(1, c);
        charSamplerDesc.minFilter = Resources::Sampler::Filter::LINEAR;
        charSamplerDesc.magFilter = Resources::Sampler::Filter::LINEAR;
        charSamplerDesc.wrapS = Resources::Sampler::WrapMode::CLAMP_TO_EDGE;
        charSamplerDesc.wrapT = Resources::Sampler::WrapMode::CLAMP_TO_EDGE;

        auto& charSampler = resourceManager->createSampler(charSamplerDesc);


        Resources::TextureDesc charTextureDesc;
        charTextureDesc.name = "texture_" + std::string(1, c);
        charTextureDesc.format = GL_RED;
        charTextureDesc.p_images[0] = &charImage;
        charTextureDesc.p_sampler = &charSampler;

        auto& charTexture = resourceManager->createTexture(charTextureDesc);


        FreeTypeCharacter character = {
            charTexture.handle,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        freeTypeChars_.insert(std::pair<char, FreeTypeCharacter>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    glGenVertexArrays(1, &VAOTextQuad_);
    glGenBuffers(1, &VBOTextQuad_);
    glBindVertexArray(VAOTextQuad_);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTextQuad_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    Resources::ShaderDesc shdrDesc;
    shdrDesc.name = TEXT_RENDERING_SHADER_NAME;
    shdrDesc.vertFilename = "../shaders/RenderText.vert";
    shdrDesc.fragFilename = "../shaders/RenderText.frag";
    auto& shdr = resourceManager->createShader(shdrDesc);

    shdr.use();
    shdr.setInt("uTextSampler", 0);

    return true;
}

void SceneManager::setTextProjectionMatrix(const glm::mat4 proj) {
    textProjMat_ = proj;

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& shdr = resourceManager->getShader(TEXT_RENDERING_SHADER_NAME);
    shdr.use();
    shdr.setMat4("uProj", textProjMat_);
}

void SceneManager::drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& shdr = resourceManager->getShader(TEXT_RENDERING_SHADER_NAME);
    shdr.use();
    shdr.setVec3("uTextColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAOTextQuad_);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        FreeTypeCharacter ch = freeTypeChars_[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        auto resource = resourceManager->getResource(ch.textureHandle);
        if (resource->type != Resources::RenderResource::ResourceType::TEXTURE) {
            LOG_E("FreeType: invalid resource bound for character \'%c\'", c);
            continue;
        }

        auto texture = static_cast<Resources::Texture*>(resource);

        glBindTexture(GL_TEXTURE_2D, texture->GL_id);
        glBindSampler(0, texture->sampler->GL_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBOTextQuad_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);

    glDisable(GL_BLEND);
}

}
