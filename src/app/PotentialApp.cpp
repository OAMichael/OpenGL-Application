#include "PotentialApp.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "JSONImporter.hpp"
#include "FileManager.hpp"
#include "Light.hpp"
#include "Logger.hpp"


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>


void PotentialApp::OnInit() {
    auto fileManager = FileSystem::FileManager::getInstance();
#ifdef __ANDROID__
    std::string basedir = "/sdcard/Android/data/com.opengl.native_activity/files";
#else
    std::string basedir = fileManager->getAbsolutePath(fileManager->getCurrentDirectory() + "/../../../..");
#endif
    fileManager->setBasedir(basedir);
    fileManager->registerProtocol("configs", basedir + "/configs");
    fileManager->registerProtocol("fonts", basedir + "/fonts");
    fileManager->registerProtocol("models", basedir + "/models");
    fileManager->registerProtocol("shaders", basedir + "/shaders");
    fileManager->registerProtocol("textures", basedir + "/textures");
}

void PotentialApp::OnWindowCreate() {
    glViewport(0, 0, windowWidth_, windowHeight_);
    Camera_.setAspect((float) windowWidth_ / windowHeight_);

    needToRender_ = windowWidth_ > 1 && windowHeight_ > 1;

    auto resourceManager = Resources::ResourceManager::getInstance();
    if (resourceManager->hasFramebuffer(Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER])) {
        resourceManager->resizeFramebuffer(Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER], windowWidth_, windowHeight_);
    }
}

void PotentialApp::OnRenderingStart() {
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    resourceManager->Init();
    auto& previewTexture = resourceManager->createTexture(fileManager->getAbsolutePath("textures://PreviewScreen.jpg"));
    previewTextureHandle_ = previewTexture.handle;

    sceneManager->createPreviewScreen();
}

void PotentialApp::OnRenderFrame() {
    processMovement();

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();

    if (ReadyForRender_) {
        ++frameAfterInit_;
        auto& modelShader = resourceManager->getShader(modelShaderHandle_);

        this->showFPS();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearDepth(1.0);

        Camera_.updateMatrices();

        Matrices ubo;
        ubo.view = Camera_.getView();
        ubo.proj = Camera_.getProj();
        ubo.model = glm::mat4(1.0f);

        //resourceManager->updateBuffer("Matrices", (const unsigned char*)&ubo, sizeof(ubo));

        //sceneManager->drawEnvironment();
        sceneManager->drawSDFScene(glm::inverse(Camera_.getView()), windowWidth_, windowHeight_, frameAfterInit_);
        /*
        std::string allStr = " ! \" # $ % & \' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @"
                             " A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \\ ] ^ _ `"
                             " a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~";
        sceneManager->drawTextFT(allStr, 200, 600, fontScale_, glm::vec3(0.8f, 0.2f, 0.5f));
        sceneManager->drawTextSDF(allStr, 200, 400, fontScale_, glm::vec3(0.8f, 0.2f, 0.5f));
        sceneManager->drawTextMSDF(allStr, 200, 200, fontScale_, glm::vec3(0.8f, 0.2f, 0.5f));

        sceneManager->drawTextFT("FT", 50, 600, 0.7, glm::vec3(0.8f, 0.9f, 0.5f));
        sceneManager->drawTextFT("SDF", 50, 400, 0.7, glm::vec3(0.8f, 0.9f, 0.5f));
        sceneManager->drawTextFT("MSDF", 50, 200, 0.7, glm::vec3(0.8f, 0.9f, 0.5f));
        */
        if (frameAfterInit_ < 100) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            sceneManager->drawPreviewScreen(previewTextureHandle_, 1.0f - frameAfterInit_ / 100.0f);
        }
    }
    else {
        sceneManager->drawPreviewScreen(previewTextureHandle_);
        if (NeedInit_) {
            //this->initModels();
            this->initLights();
            this->initRender();
            this->initCamera();
            ReadyForRender_ = true;
        }
        NeedInit_ = true;
    }
}

void PotentialApp::OnRenderingEnd() {
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();

    resourceManager->cleanUp();
    sceneManager->cleanUp();
}

void PotentialApp::OnWindowDestroy() {

}


#ifdef __ANDROID__
int32_t PotentialApp::handleInputCallback(android_app* app, AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);
    int32_t action = AMotionEvent_getAction(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        static int32_t startPointerId = 0;
        static int32_t startX = 0;
        static int32_t startY = 0;

        switch (action) {
            case AMOTION_EVENT_ACTION_MOVE: {
                int32_t horizontalDistance = 0;
                int32_t verticalDistance = 0;

                if (startPointerId == AMotionEvent_getPointerId(event, 0)) {
                    horizontalDistance = AMotionEvent_getX(event, 0) - startX;
                    verticalDistance = AMotionEvent_getY(event, 0) - startY;
                }

                const float sensitivity = Camera_.getSensitivity();
                horizontalDistance *= sensitivity;
                verticalDistance *= sensitivity;

                Camera_.setYaw(Camera_.getYaw() + horizontalDistance);
                Camera_.setPitch(std::clamp(Camera_.getPitch() - verticalDistance, -89.0f, 89.0f));
                Camera_.updateVectors();

                startPointerId = AMotionEvent_getPointerId(event, 0);
                startX = AMotionEvent_getX(event, 0);
                startY = AMotionEvent_getY(event, 0);
                return 1;
            }
            case AMOTION_EVENT_ACTION_UP: {
                return 1;
            }
            case AMOTION_EVENT_ACTION_DOWN: {
                startPointerId = AMotionEvent_getPointerId(event, 0);
                startX = AMotionEvent_getX(event, 0);
                startY = AMotionEvent_getY(event, 0);
                return 1;
            }
        }
    }
    return 0;
}
#else
void PotentialApp::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if(action == GLFW_PRESS) {
                MouseHidden_ = !MouseHidden_;

                if(MouseHidden_)
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                else
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            break;
    }
}


void PotentialApp::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    if(FirstMouse_) {
        MouseLastX_ = xpos;
        MouseLastY_ = ypos;
        FirstMouse_ = false;
    }

    float xoffset = xpos - MouseLastX_;
    float yoffset = MouseLastY_ - ypos;
    MouseLastX_ = xpos;
    MouseLastY_ = ypos;

    if(MouseHidden_) {
        const float sensitivity = Camera_.getSensitivity();
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        Camera_.setYaw(Camera_.getYaw() + xoffset);
        Camera_.setPitch(std::clamp(Camera_.getPitch() + yoffset, -89.0f, 89.0f));
        Camera_.updateVectors();
    }
}


void PotentialApp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {

}


void PotentialApp::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth_  = width;
    windowHeight_ = height;
    Camera_.setAspect((float)width / height);

    needToRender_ = width > 1 && height > 1;

    auto resourceManager = Resources::ResourceManager::getInstance();
    resourceManager->resizeFramebuffer(Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER], width, height);

    auto sceneManager = SceneResources::SceneManager::getInstance();
    sceneManager->setTextProjectionMatrix(glm::ortho(0.0f, (float)windowWidth_, 0.0f, (float)windowHeight_));
}


void PotentialApp::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(MouseHidden_) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            keyPressedA_ = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
            keyPressedA_ = false;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            keyPressedW_ = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
            keyPressedW_ = false;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            keyPressedS_ = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
            keyPressedS_ = false;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            keyPressedD_ = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
            keyPressedD_ = false;
        }
    }

    switch(key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_F11:
            if(action == GLFW_PRESS) {
                IsFullscreen_ = !IsFullscreen_;

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                if(IsFullscreen_) {
                    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                }
                else {
                    glfwSetWindowMonitor(window, nullptr, 0, 0, 1200, 900, 0);
                }
            }
            break;

        case GLFW_KEY_M:
            if (action == GLFW_PRESS) {
                IsWireframe_ = !IsWireframe_;
            }
            break;

        case GLFW_KEY_LEFT_CONTROL:
            if(action == GLFW_PRESS)
                Camera_.setFov(30.0f);
            if(action == GLFW_RELEASE)
                Camera_.setFov(90.0f);
            break;

        case GLFW_KEY_LEFT_SHIFT:
            if(action == GLFW_PRESS) {
                Camera_.setDirSpeed(2.5f);
                Camera_.setRightSpeed(1.8f);
            }
            if(action == GLFW_RELEASE) {
                Camera_.setDirSpeed(1.0f);
                Camera_.setRightSpeed(1.0f);
            }
            break;

        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS) {
                auto sceneManager = SceneResources::SceneManager::getInstance();
                auto env = sceneManager->getEnvironmentType();
                env = static_cast<SceneResources::SceneManager::EnvironmentType>(
                    (env + 1) % SceneResources::SceneManager::EnvironmentType::COUNT);
                sceneManager->setEnvironmentType(env);
            }
            break;

        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS) {
                auto sceneManager = SceneResources::SceneManager::getInstance();
                auto env = sceneManager->getEnvironmentType();
                env = static_cast<SceneResources::SceneManager::EnvironmentType>(
                    (env + SceneResources::SceneManager::EnvironmentType::COUNT - 1) % SceneResources::SceneManager::EnvironmentType::COUNT);
                sceneManager->setEnvironmentType(env);
            }
            break;

        case GLFW_KEY_B:
            if (action == GLFW_PRESS) {
                auto sceneManager = SceneResources::SceneManager::getInstance();
                sceneManager->setEnableBloom(!sceneManager->getEnableBloom());
            }
            break;
        case GLFW_KEY_EQUAL:
            if (action == GLFW_PRESS) {
                fontScale_ *= 1.05f;
            }
            break;
        case GLFW_KEY_MINUS:
            if (action == GLFW_PRESS) {
                fontScale_ /= 1.05f;
            }
            break;

    }

}
#endif


PotentialApp::PotentialApp() {
    m_Application = this;

    enableDefaultMSAA_ = false;// true;
    samples_ = 1;
}


void PotentialApp::initModels() {
    auto fileManager = FileSystem::FileManager::getInstance();
    auto GLTFloader = GLTF::GLTFLoader::getInstance();
    auto jsonImporter = JsonUtil::JSONImpoter::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();
    auto resourceManager = Resources::ResourceManager::getInstance();

    std::ifstream modelsConfig { fileManager->getAbsolutePath(CONFIG_PATH) };
    nlohmann::json cfg;
    modelsConfig >> cfg;

    std::vector<JsonUtil::JSONImpoter::ModelImportInfo> modelsInfo;
    jsonImporter->loadModelsInfo(cfg, modelsInfo);

    auto& rootNode = sceneManager->createRootNode();

    Models_.resize(modelsInfo.size());
    for (int i = 0; i < Models_.size(); ++i) {
        auto& modelInfo = modelsInfo[i];
        auto& newModel = Models_[i];

        newModel.setName(modelInfo.name);
        newModel.setFilename(modelInfo.filename);
        newModel.setPosition(modelInfo.position);
        newModel.setRotation(modelInfo.rotation);
        newModel.setScale(modelInfo.scale);

        GLTFloader->load(newModel, newModel.getFilename());
        newModel.init();
        newModel.getModelRootNode()->setParent(&rootNode);
    }
    rootNode.printNode();
}


void PotentialApp::initLights() {
    auto sceneManager = SceneResources::SceneManager::getInstance();
    auto resourceManager = Resources::ResourceManager::getInstance();

    glm::vec4 lightColor = glm::vec4(1.0f);
    std::array<glm::vec4, 3> pointLightPositions = {
        glm::vec4(4.0f, 0.0f, -3.0f, 0.0f),
        glm::vec4(4.0f, 0.0f, 3.0f, 0.0f),
        glm::vec4(-3.0f, 0.5f, 0.0f, 0.0f)
    };

    std::array<glm::vec4, 6> directionalLightDirections = {
        glm::vec4(1.0f,  0.0f, 0.0f, 0.0f),
        glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f,  1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f,  0.0f, 1.0f, 0.0f),
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)
    };

    std::array<glm::vec3, 2> spotLightPositions = {
        glm::vec3(0.0f),
        glm::vec3(0.0f)
    };
    std::array<glm::vec4, 2> spotLightDirectionsCutoffs = {
        glm::vec4(1.0f, 0.0f, 0.0f, glm::cos(glm::radians(10.0f))),
        glm::vec4(-1.0f, 0.0f, 1.0f, glm::cos(glm::radians(20.0f)))
    };


    for (int i = 0; i < pointLightPositions.size(); ++i) {
        SceneResources::LightDesc lightDesc;
        lightDesc.color = lightColor;
        lightDesc.position = pointLightPositions[i];
        lightDesc.type = SceneResources::SceneLight::LightType::POINT_LIGHT;
        sceneManager->createSceneLight(lightDesc);
    }
    for (int i = 0; i < directionalLightDirections.size(); ++i) {
        SceneResources::LightDesc lightDesc;
        lightDesc.color = lightColor;
        lightDesc.direction = directionalLightDirections[i];
        lightDesc.type = SceneResources::SceneLight::LightType::DIRECTIONAL_LIGHT;
        sceneManager->createSceneLight(lightDesc);
    }
    for (int i = 0; i < spotLightPositions.size(); ++i) {
        SceneResources::LightDesc lightDesc;
        lightDesc.color = lightColor;
        lightDesc.position = spotLightPositions[i];
        lightDesc.direction = spotLightDirectionsCutoffs[i].xyz();
        lightDesc.cutoff_angle = spotLightDirectionsCutoffs[i].w;
        lightDesc.type = SceneResources::SceneLight::LightType::SPOT_LIGHT;
        sceneManager->createSceneLight(lightDesc);
    }

    sceneManager->updateLights();
    const auto& lightDataBuff = sceneManager->getLightData();
    
    Resources::BufferDesc lightsBufDesc;
    lightsBufDesc.name = "Lights";
    lightsBufDesc.uri = "";
    lightsBufDesc.bytesize = sizeof(SceneResources::LightData);
    lightsBufDesc.target = GL_SHADER_STORAGE_BUFFER;
    lightsBufDesc.p_data = (const unsigned char*)(&lightDataBuff);

    resourceManager->createBuffer(lightsBufDesc);
}


void PotentialApp::initRender() {
    auto sceneManager = SceneResources::SceneManager::getInstance();
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto fileManager = FileSystem::FileManager::getInstance();

    Resources::ShaderDesc shaderDesc;
    shaderDesc.name = MODEL_SHADER_NAME;
    shaderDesc.uri = "";
    shaderDesc.vertFilename = fileManager->getAbsolutePath("shaders://Model.vert");
    shaderDesc.fragFilename = fileManager->getAbsolutePath("shaders://Model.frag");

    auto& modelShader = resourceManager->createShader(shaderDesc);
    modelShaderHandle_ = modelShader.handle;

    const std::vector<std::string> background2DTexturesNames = {
        fileManager->getAbsolutePath("textures://city.jpg")
    };

    const std::vector<std::string> skyboxTexturesNames = {
        fileManager->getAbsolutePath("textures://posx.jpg"),
        fileManager->getAbsolutePath("textures://negx.jpg"),
        fileManager->getAbsolutePath("textures://posy.jpg"),
        fileManager->getAbsolutePath("textures://negy.jpg"),
        fileManager->getAbsolutePath("textures://posz.jpg"),
        fileManager->getAbsolutePath("textures://negz.jpg")
    };

    const std::vector<std::string> equirectTexturesNames = {
        fileManager->getAbsolutePath("textures://bethnal_green_entrance_4k.hdr")
    };

    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::BACKGROUND_IMAGE_2D, background2DTexturesNames);
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::SKYBOX, skyboxTexturesNames);
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::EQUIRECTANGULAR, equirectTexturesNames, true);

    sceneManager->createImageBasedLightingTextures(SceneResources::SceneManager::EnvironmentType::SKYBOX);
    sceneManager->createImageBasedLightingTextures(SceneResources::SceneManager::EnvironmentType::EQUIRECTANGULAR);

    Resources::BufferDesc bufDesc;
    bufDesc.name = "Matrices";
    bufDesc.uri = "";
    bufDesc.bytesize = sizeof(Matrices);
    bufDesc.target = GL_UNIFORM_BUFFER;
    bufDesc.p_data = nullptr;

    resourceManager->createBuffer(bufDesc);

    auto& envShader = resourceManager->getShader(SceneResources::ENVIRONMENT_SHADER_NAME);

    resourceManager->bindBufferShader("Matrices", 0, modelShader);
    resourceManager->bindBufferShader("Matrices", 0, envShader);

    resourceManager->bindBufferShader("Lights", 1, modelShader);


    Resources::ImageDesc fbImageDesc;
    fbImageDesc.name = MODEL_FRAMEBUFFER_NAME + std::string("_IMAGE");
    fbImageDesc.uri = "";
    fbImageDesc.width = windowWidth_;
    fbImageDesc.height = windowHeight_;
    fbImageDesc.components = 4;
    fbImageDesc.bits = 8 * sizeof(float);
    fbImageDesc.format = GL_RGBA;
    fbImageDesc.p_data = nullptr;
    Resources::Image& fbImage = resourceManager->createImage(fbImageDesc);

    Resources::TextureDesc fbTextureDesc;
    fbTextureDesc.name = MODEL_FRAMEBUFFER_TEXTURE_NAME;
    fbTextureDesc.uri = "";
    fbTextureDesc.format = GL_RGBA16F;
    fbTextureDesc.p_images[0] = &fbImage;
    Resources::Texture& fbTexture = resourceManager->createTexture(fbTextureDesc);

    fbTextureDesc.name = MODEL_FRAMEBUFFER_TEXTURE_NAME + std::string("_DEPTH");
    fbTextureDesc.uri = "";
    fbTextureDesc.format = GL_DEPTH_COMPONENT32F;
    fbTextureDesc.p_images[0] = &fbImage;
    Resources::Texture& fbTextureDepth = resourceManager->createTexture(fbTextureDesc);

    Resources::FramebufferDesc fbDesc;
    fbDesc.name = MODEL_FRAMEBUFFER_NAME;
    fbDesc.uri = "";
    fbDesc.colorAttachmentsCount = 1;
    fbDesc.colorAttachments[0] = &fbTexture;
    fbDesc.depthAttachment = &fbTextureDepth;
    fbDesc.dependency = Resources::defaultFramebufferNames[Resources::Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER];
    Resources::Framebuffer& fb = resourceManager->createFramebuffer(fbDesc);

    modelFramebufferHandle_ = fb.handle;
    modelFramebufferTextureHandle_ = fb.colorAttachments[0]->handle;

    SceneResources::SceneManager::PostProcessInfo ppi;
    ppi.enableBlur = false;
    ppi.enableBloom = false;
    ppi.windowWidth = windowWidth_;
    ppi.windowHeight = windowHeight_;
    sceneManager->createPostProcess(ppi);

    sceneManager->initializeFreeType();
    sceneManager->initializeFTTextRendering(fileManager->getAbsolutePath("fonts://arial.ttf"));
    sceneManager->initializeSDFTextRendering(fileManager->getAbsolutePath("fonts://arial.ttf"));
    sceneManager->initializeMSDFTextRendering(fileManager->getAbsolutePath("fonts://arial.ttf"));
    sceneManager->setTextProjectionMatrix(glm::ortho(0.0f, (float)windowWidth_, 0.0f, (float)windowHeight_));

    sceneManager->initializeSDFScene();
}


void PotentialApp::initCamera() {
    Camera_.setAspect((float)windowWidth_ / windowHeight_);
    Camera_.setZNear(0.01f);
    Camera_.setZFar(100.0f);
}


void PotentialApp::processMovement() {
#ifndef __ANDROID__
    if (!MouseHidden_) {
        return;
    }

    if (keyPressedA_) {
        Camera_.moveRight(-2 * deltaTime_);
    }
    if (keyPressedW_) {
        Camera_.moveFront(2 * deltaTime_);
    }
    if (keyPressedS_) {
        Camera_.moveFront(-2 * deltaTime_);
    }
    if (keyPressedD_) {
        Camera_.moveRight(2 * deltaTime_);
    }
#endif
}
