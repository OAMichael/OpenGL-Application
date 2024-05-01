#include "SceneManager.hpp"
#include "ResourceManager.hpp"
#include "FileManager.hpp"
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
    initializeDefaultQuad();

    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::TextureDesc texDesc;
    texDesc.faces = 1;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = BACKGROUND_2D_TEXTURE_NAME;

    auto& newImage = resourceManager->createImage(textureName);
    texDesc.p_images[0] = &newImage;
    texDesc.format = resourceManager->chooseDefaultInternalFormat(newImage.components, isHdr);

    Background2DHandle_ = resourceManager->createTexture(texDesc).handle;
}

void SceneManager::drawBackground2D() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    resourceManager->bindTexture(Background2DHandle_, 0);
    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK], 1);
    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK], 2);

    drawDefaultQuad();

    glDepthFunc(GL_LESS);
}


void SceneManager::createSkybox(const std::vector<std::string>& textureNames, bool isHdr) {
    initializeDefaultCube();

    auto resourceManager = Resources::ResourceManager::getInstance();

    unsigned faces = 6;
    Resources::TextureDesc texDesc;
    texDesc.faces = 6;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = SKYBOX_TEXTURE_NAME;

    int components = 0;
    for (unsigned i = 0; i < faces; ++i) {
        auto& newImage = resourceManager->createImage(textureNames[i]);
        texDesc.p_images[i] = &newImage;
        components = newImage.components;
    }
    texDesc.p_sampler = &resourceManager->getSampler(Resources::Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_MIPMAP_LINEAR_CLAMP);
    texDesc.format = resourceManager->chooseDefaultInternalFormat(components, isHdr);

    SkyboxHandle_ = resourceManager->createTexture(texDesc).handle;
    resourceManager->generateMipMaps(SkyboxHandle_);

#ifndef __ANDROID__
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
}

void SceneManager::drawSkybox() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK], 0);
    resourceManager->bindTexture(SkyboxHandle_, 1);
    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK], 2);

    drawDefaultCube();

    glDepthFunc(GL_LESS);
}


void SceneManager::createEquirectangular(const std::string& textureName, bool isHdr) {
    initializeDefaultCube();

    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::TextureDesc texDesc;
    texDesc.faces = 1;
    texDesc.factor = glm::vec4(1.0);
    texDesc.name = EQUIRECTANGULAR_TEXTURE_NAME;

    auto& newImage = resourceManager->createImage(textureName, isHdr);
    texDesc.p_images[0] = &newImage;
    texDesc.format = resourceManager->chooseDefaultInternalFormat(newImage.components, isHdr);

    EquirectHandle_ = resourceManager->createTexture(texDesc).handle;
    resourceManager->generateMipMaps(EquirectHandle_);
}

void SceneManager::drawEquirectangular() {
    auto resourceManager = Resources::ResourceManager::getInstance();

    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CCW);

    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK], 0);
    resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK], 1);
    resourceManager->bindTexture(EquirectHandle_, 2);

    drawDefaultCube();

    glDepthFunc(GL_LESS);
}


void SceneManager::createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames, bool isHdr) {
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = ENVIRONMENT_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://DefaultEnv.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://DefaultEnv.frag");

    auto& envShader = resourceManager->createShader(shaderDesc);
    environmentShaderHandle_ = envShader.handle;

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

void SceneManager::createImageBasedLightingTextures(const EnvironmentType envType) {
    if (envType != EnvironmentType::SKYBOX && envType != EnvironmentType::EQUIRECTANGULAR) {
        LOG_W("Irradiance map is only supported for skybox and equirectangular environments");
        return;
    }

    if (envType == EnvironmentType::SKYBOX && !SkyboxHandle_.isValid()) {
        LOG_E("Cannot create IBL textures for non existing skybox environment");
        return;
    }

    if (envType == EnvironmentType::EQUIRECTANGULAR && !EquirectHandle_.isValid()) {
        LOG_E("Cannot create IBL textures for non existing equirectangular environment");
        return;
    }

    initializeDefaultQuad();
    initializeDefaultCube();

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;

    shaderDesc.name = IRRADIANCE_MAP_SHADER_NAME;
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://DefaultCubemap.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://IBL/IrradianceMap.frag");
    auto& irradianceMapShader = resourceManager->createShader(shaderDesc);

    shaderDesc.name = PREFILTER_HDR_SHADER_NAME;
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://DefaultCubemap.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://IBL/PrefilterHDRMap.frag");
    auto& prefilterHDRShader = resourceManager->createShader(shaderDesc);

    shaderDesc.name = BRDF_LUT_SHADER_NAME;
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://IBL/BRDF_LUT.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://IBL/BRDF_LUT.frag");
    auto& brdfLUTShader = resourceManager->createShader(shaderDesc);


    Resources::ImageDesc imageDesc;
    imageDesc.format = GL_RGB;
    imageDesc.components = 3;
    imageDesc.bits = 8 * sizeof(float);
    imageDesc.p_data = nullptr;

    imageDesc.name = "IRRADIANCE_FACE_IMAGE";
    imageDesc.width = irradianceMapSize_;
    imageDesc.height = irradianceMapSize_;
    auto& irradianceFaceImage = resourceManager->createImage(imageDesc);

    imageDesc.name = "PREFILTER_HDR_FACE_IMAGE";
    imageDesc.width = prefilteredHDRMapSize_;
    imageDesc.height = prefilteredHDRMapSize_;
    auto& prefilterHDRFaceImage = resourceManager->createImage(imageDesc);

    imageDesc.name = "BRDF_LUT_IMAGE";
    imageDesc.width = brdfLUTSize_;
    imageDesc.height = brdfLUTSize_;
    imageDesc.format = GL_RG;
    imageDesc.components = 2;
    auto& brdfLUTImage = resourceManager->createImage(imageDesc);


    Resources::TextureDesc textureDesc;
    textureDesc.faces = 6;
    textureDesc.format = GL_RGB16F;

    textureDesc.name = std::string("IRRADIANCE_FACE_TEXTURE_") + (envType == EnvironmentType::SKYBOX ? "SKYBOX" : "EQUIRECT");
    textureDesc.p_sampler = &resourceManager->getSampler(Resources::Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_CLAMP);
    for (int i = 0; i < 6; ++i) {
        textureDesc.p_images[i] = &irradianceFaceImage;
    }
    auto& irradianceMapTex = resourceManager->createTexture(textureDesc);

    textureDesc.name = std::string("PREFILTER_HDR_FACE_TEXTURE_") + (envType == EnvironmentType::SKYBOX ? "SKYBOX" : "EQUIRECT");
    textureDesc.p_sampler = &resourceManager->getSampler(Resources::Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_MIPMAP_LINEAR_CLAMP);
    for (int i = 0; i < 6; ++i) {
        textureDesc.p_images[i] = &prefilterHDRFaceImage;
    }
    auto& prefilterHDRTex = resourceManager->createTexture(textureDesc);
    resourceManager->generateMipMaps(prefilterHDRTex.handle);

    textureDesc.name = std::string("BRDF_LUT_TEXTURE_") + (envType == EnvironmentType::SKYBOX ? "SKYBOX" : "EQUIRECT");
    textureDesc.p_sampler = &resourceManager->getSampler(Resources::Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_CLAMP);
    textureDesc.faces = 1;
    textureDesc.format = GL_RG16F;
    textureDesc.p_images[0] = &brdfLUTImage;
    auto& brdfLUTTexture = resourceManager->createTexture(textureDesc);


    if (envType == EnvironmentType::SKYBOX) {
        irradianceMapSkyboxTextureHandle_ = irradianceMapTex.handle;
        prefilterHDRSkyboxTextureHandle_ = prefilterHDRTex.handle;
        brdfLUTSkyboxTextureHandle_ = brdfLUTTexture.handle;
    }
    else {
        irradianceMapEquirectTextureHandle_ = irradianceMapTex.handle;
        prefilterHDREquirectTextureHandle_ = prefilterHDRTex.handle;
        brdfLUTEquirectTextureHandle_ = brdfLUTTexture.handle;
    }


    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    unsigned framebufferIBL;
    unsigned renderbufferIBL;

    glGenFramebuffers(1, &framebufferIBL);
    glGenRenderbuffers(1, &renderbufferIBL);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferIBL);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferIBL);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceMapSize_, irradianceMapSize_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferIBL);

    irradianceMapShader.use();
    irradianceMapShader.setInt("uSamplerSkybox", 0);
    irradianceMapShader.setInt("uSamplerEquirect", 1);
    irradianceMapShader.setUint("uEnvironmentType", envType);
    irradianceMapShader.setMat4("proj", captureProjection);

    Resources::Texture* cubemap = nullptr;
    if (envType == EnvironmentType::SKYBOX) {
        cubemap = &resourceManager->getTexture(SkyboxHandle_);
    }
    else {
        cubemap = &resourceManager->getTexture(Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK);
    }
    resourceManager->bindTexture(cubemap->handle, 0);

    Resources::Texture* equirect = nullptr;
    if (envType == EnvironmentType::SKYBOX) {
        equirect = &resourceManager->getTexture(Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK);
    }
    else {
        equirect = &resourceManager->getTexture(EquirectHandle_);
    }
    resourceManager->bindTexture(equirect->handle, 1);


    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glViewport(0, 0, irradianceMapSize_, irradianceMapSize_);
    for (unsigned int i = 0; i < 6; ++i) {
        irradianceMapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMapTex.GL_id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawDefaultCube();
    }



    prefilterHDRShader.use();
    prefilterHDRShader.setInt("uSamplerSkybox", 0);
    prefilterHDRShader.setInt("uSamplerEquirect", 1);
    prefilterHDRShader.setUint("uEnvironmentType", envType);
    prefilterHDRShader.setMat4("proj", captureProjection);

    for (unsigned int mip = 0; mip < maxMipLevelsPrefilterHDR_; ++mip) {
        unsigned int mipWidth = prefilteredHDRMapSize_ * std::pow(0.5, mip);
        unsigned int mipHeight = prefilteredHDRMapSize_ * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferIBL);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevelsPrefilterHDR_ - 1);
        prefilterHDRShader.setFloat("uRoughness", roughness);
        for (unsigned int i = 0; i < 6; ++i) {
            prefilterHDRShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterHDRTex.GL_id, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawDefaultCube();
        }
    }



    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdfLUTSize_, brdfLUTSize_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture.GL_id, 0);

    brdfLUTShader.use();
    glViewport(0, 0, brdfLUTSize_, brdfLUTSize_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawDefaultQuad();

    resourceManager->generateMipMaps(brdfLUTTexture.handle);


    resourceManager->bindFramebuffer(Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER]);
    glDeleteFramebuffers(1, &framebufferIBL);
    glDeleteRenderbuffers(1, &renderbufferIBL);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void SceneManager::drawEnvironment() {
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& envShader = resourceManager->getShader(environmentShaderHandle_);
    envShader.use();
    envShader.setUint("uEnvironmentType", (uint32_t)envType_);
    envShader.setBool("uIsHdr", envHdr_[envType_]);

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
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = GAUSSIAN_BLUR_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://PostProcess/GaussianBlur.frag");

    auto& blurShader = resourceManager->createShader(shaderDesc);
    gaussianBlurShaderHandle_ = blurShader.handle;
    blurShader.use();
    blurShader.setInt("uScreenTexture", 0);


    shaderDesc.name = BLOOM_SHADER_NAME;
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://PostProcess/Bloom.frag");

    auto& bloomShader = resourceManager->createShader(shaderDesc);
    bloomShaderHandle_ = bloomShader.handle;
    bloomShader.use();
    bloomShader.setInt("uScreenTexture", 0);


    shaderDesc.name = BLOOM_FINAL_SHADER_NAME;
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://PostProcess/BloomFinal.frag");

    auto& bloomFinalShader = resourceManager->createShader(shaderDesc);
    bloomFinalShaderHandle_ = bloomFinalShader.handle;
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
    fbDesc.dependency = Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER];


    /* Create Gaussian Blur X */
    {
        fbImageDesc.name = "GAUSSIAN_BLUR_X_IMAGE";
        Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_X_TEXTURE";
        fbTextureDesc.format = GL_RGBA16F;
        fbTextureDesc.p_images[0] = &fbImage;
        Resources::Texture& fbTexture = resourceManager->createTexture(fbTextureDesc);

        fbTextureDesc.name = "GAUSSIAN_BLUR_X_TEXTURE_DEPTH";
        fbTextureDesc.format = GL_DEPTH_COMPONENT32F;
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
        fbTextureDesc.format = GL_DEPTH_COMPONENT32F;
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
        fbTextureDesc.format = GL_DEPTH_COMPONENT32F;
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
        fbTextureDesc.format = GL_DEPTH_COMPONENT32F;
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
        auto& bloomShader = resourceManager->getShader(bloomShaderHandle_);
        auto& blurShader = resourceManager->getShader(gaussianBlurShaderHandle_);
        auto& bloomFinalShader = resourceManager->getShader(bloomFinalShaderHandle_);

        auto& bloomFramebuffer = resourceManager->getFramebuffer(bloomFramebufferHandle_);
        auto& blurXFramebuffer = resourceManager->getFramebuffer(blurXFramebufferHandle_);
        auto& blurYFramebuffer = resourceManager->getFramebuffer(blurYFramebufferHandle_);
        auto& bloomFinalFramebuffer = resourceManager->getFramebuffer(bloomFinalFramebufferHandle_);

        auto* bloomColorTexture = bloomFramebuffer.colorAttachments[0];
        auto* bloomBrightnessTexture = bloomFramebuffer.colorAttachments[1];
        auto* blurXTexture = blurXFramebuffer.colorAttachments[0];
        auto* blurYTexture = blurYFramebuffer.colorAttachments[0];
        auto* bloomFinalTexture = bloomFinalFramebuffer.colorAttachments[0];


        resourceManager->bindFramebuffer(bloomFramebufferHandle_);
        drawFullscreenQuad(postProcessTextureHandle_, &bloomShader);

        blurShader.use();
        resourceManager->bindFramebuffer(blurXFramebufferHandle_);
        blurShader.setBool("uHorizontal", true);
        drawFullscreenQuad(bloomBrightnessTexture->handle, &blurShader);

        resourceManager->bindFramebuffer(blurYFramebufferHandle_);
        blurShader.setBool("uHorizontal", false);
        drawFullscreenQuad(blurXTexture->handle, &blurShader);

        resourceManager->bindFramebuffer(bloomFinalFramebufferHandle_);
        resourceManager->bindTexture(blurYTexture->handle, 1);
        drawFullscreenQuad(bloomColorTexture->handle, &bloomFinalShader);        // Setups only texture0

        postProcessTextureHandle_ = bloomFinalTexture->handle;
    }

    if (postProcessInfo_.enableBlur) {
        auto& blurShader = resourceManager->getShader(gaussianBlurShaderHandle_);

        auto& blurXFramebuffer = resourceManager->getFramebuffer(blurXFramebufferHandle_);
        auto& blurYFramebuffer = resourceManager->getFramebuffer(blurYFramebufferHandle_);

        auto& inputTexture = resourceManager->getTexture(postProcessTextureHandle_);
        auto* blurXTexture = blurXFramebuffer.colorAttachments[0];
        auto* blurYTexture = blurYFramebuffer.colorAttachments[0];


        blurShader.use();
        resourceManager->bindFramebuffer(blurXFramebufferHandle_);
        blurShader.setBool("uHorizontal", true);
        drawFullscreenQuad(postProcessTextureHandle_, &blurShader);

        resourceManager->bindFramebuffer(blurYFramebufferHandle_);
        blurShader.setBool("uHorizontal", false);
        drawFullscreenQuad(blurXTexture->handle, &blurShader);

        postProcessTextureHandle_ = blurYTexture->handle;
    }
}

void SceneManager::createFullscreenQuad() {
    auto* resourceManager = Resources::ResourceManager::getInstance();
    auto* fileManager = FileSystem::FileManager::getInstance();

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
    shdrDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shdrDesc.fragFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.frag");
    auto& shdr = resourceManager->createShader(shdrDesc);
    fullscreenShaderHandle_ = shdr.handle;

    shdr.use();
    shdr.setInt("uScreenTexture", 0);
}

void SceneManager::drawFullscreenQuad(const Resources::ResourceHandle inputTextureHandle, Resources::Shader* shader) {
    // Assume that proper framebuffer already bound
    auto* resourceManager = Resources::ResourceManager::getInstance();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (shader) {
        shader->use();
    }
    else {
        resourceManager->getShader(fullscreenShaderHandle_).use();
    }

    resourceManager->bindTexture(inputTextureHandle);
    glBindVertexArray(VAOFullscreenQuad_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SceneManager::drawToDefaultFramebuffer(const Resources::ResourceHandle inputTextureHandle) {
    auto* resourceManager = Resources::ResourceManager::getInstance();
    if (!resourceManager->hasShader(FULLSCREEN_QUAD_SHADER_NAME)) {
        createFullscreenQuad();
    }

    resourceManager->bindFramebuffer(Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER]);
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
#ifndef __ANDROID__
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

    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shdrDesc;
    shdrDesc.name = TEXT_RENDERING_SHADER_NAME;
    shdrDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/RenderText.vert");
    shdrDesc.fragFilename = fileManager->getAbsolutePath("shaders://PostProcess/RenderText.frag");
    auto& shdr = resourceManager->createShader(shdrDesc);
    textRenderingShaderHandle_ = shdr.handle;

    shdr.use();
    shdr.setInt("uTextSampler", 0);
#endif
    return true;
}

void SceneManager::setTextProjectionMatrix(const glm::mat4 proj) {
#ifndef __ANDROID__
    textProjMat_ = proj;

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& shdr = resourceManager->getShader(textRenderingShaderHandle_);
    shdr.use();
    shdr.setMat4("uProj", textProjMat_);
#endif
}

void SceneManager::drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
#ifndef __ANDROID__
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto& shdr = resourceManager->getShader(textRenderingShaderHandle_);
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

        auto& texture = resourceManager->getTexture(ch.textureHandle);
        resourceManager->bindTexture(ch.textureHandle);
        glBindBuffer(GL_ARRAY_BUFFER, VBOTextQuad_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glDisable(GL_BLEND);
#endif
}

void SceneManager::initializeDefaultCube() {
    if (VAODefaultCube_ != 0) {
        return;
    }

    glGenVertexArrays(1, &VAODefaultCube_);
    glGenBuffers(1, &VBODefaultCube_);
    glBindVertexArray(VAODefaultCube_);

    const std::vector<float>& vertsCube = defaultCube_.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, VBODefaultCube_);
    glBufferData(GL_ARRAY_BUFFER, vertsCube.size() * sizeof(float), vertsCube.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawDefaultCube() {
    if (VAODefaultCube_ == 0) {
        initializeDefaultCube();
    }

    glBindVertexArray(VAODefaultCube_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


void SceneManager::initializeDefaultQuad() {
    if (VAODefaultQuad_ != 0) {
        return;
    }

    glGenVertexArrays(1, &VAODefaultQuad_);
    glGenBuffers(1, &VBODefaultQuad_);
    glBindVertexArray(VAODefaultQuad_);

    const std::vector<float>& vertsQuad = {
        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,     1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBODefaultQuad_);
    glBufferData(GL_ARRAY_BUFFER, vertsQuad.size() * sizeof(float), vertsQuad.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneManager::drawDefaultQuad() {
    if (VAODefaultQuad_ == 0) {
        initializeDefaultQuad();
    }

    glBindVertexArray(VAODefaultQuad_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


void SceneManager::createPreviewScreen() {
    initializeDefaultQuad();

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = PREVIEW_SCREEN_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://PreviewScreen.frag");

    auto& previewShader = resourceManager->createShader(shaderDesc);
    previewScreenShaderHandle_  = previewShader.handle;
    previewShader.use();
    previewShader.setInt("uScreenTexture", 0);
    previewShader.setFloat("uAlpha", 1.0f);
}

void SceneManager::drawPreviewScreen(const Resources::ResourceHandle textureHandle, const float alpha) {
    auto resourceManager = Resources::ResourceManager::getInstance();

    auto& previewShader = resourceManager->getShader(previewScreenShaderHandle_);
    auto& texture = resourceManager->getTexture(textureHandle);

    previewShader.use();
    previewShader.setFloat("uAlpha", alpha);
    resourceManager->bindTexture(textureHandle);

    drawDefaultQuad();
}


void SceneManager::initializeSDFScene() {
    initializeDefaultQuad();

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = SDF_SCENE_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://PostProcess/FullscreenQuad.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://SDF/SDFScene.frag");

    Resources::SamplerDesc samplerDesc;
    samplerDesc.name = "RGBA_Noise_Sampler";
    samplerDesc.uri = "";
    samplerDesc.wrapR = Resources::Sampler::WrapMode::REPEAT;
    samplerDesc.wrapS = Resources::Sampler::WrapMode::REPEAT;
    samplerDesc.wrapT = Resources::Sampler::WrapMode::REPEAT;
    samplerDesc.minFilter = Resources::Sampler::Filter::NEAREST_MIPMAP_LINEAR;
    samplerDesc.magFilter = Resources::Sampler::Filter::LINEAR;

    auto& noiseSampler = resourceManager->createSampler(samplerDesc);

    auto& noiseTexture = resourceManager->createTexture(fileManager->getAbsolutePath("textures://rgbanoise.png"), false, &noiseSampler);
    rgbaNoiseTextureHandle_ = noiseTexture.handle;
    resourceManager->generateMipMaps(rgbaNoiseTextureHandle_);

    auto& SDFShader = resourceManager->createShader(shaderDesc);
    SDFSceneShaderHandle_ = SDFShader.handle;
    SDFShader.use();
}

void SceneManager::drawSDFScene(const glm::mat4& invCameraMatrix, const float winWidth, const float winHeight, const float time) {
    auto resourceManager = Resources::ResourceManager::getInstance();

    auto& SDFShader = resourceManager->getShader(SDFSceneShaderHandle_);
    SDFShader.use();
    SDFShader.setInt("uRGBANoiseSampler", 0);
    SDFShader.setMat4("invCameraMatrix", invCameraMatrix);
    SDFShader.setFloat("windowWidth", winWidth);
    SDFShader.setFloat("windowHeight", winHeight);
    SDFShader.setFloat("time", time);

    resourceManager->bindTexture(rgbaNoiseTextureHandle_);

    drawDefaultQuad();
}

}
