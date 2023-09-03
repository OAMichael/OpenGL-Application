#include "PotentialApp.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"

void PotentialApp::renderToWindow() {
    
    this->initRender();
    this->initCamera();

    auto GLTFloader = GLTF::GLTFLoader::getInstance();
    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();

    auto& rootNode = sceneManager->createSceneNode("RootNode");
    for (auto& model : Models_) {
        GLTFloader->load(model.getModelRef(), model.getFilename());
        model.init();
        model.getModelRootNode()->setParent(&rootNode);
    }
    rootNode.printNode();


    Matrices ubo;

    resourceManager->updateBuffer("Matrices", (const unsigned char*)&ubo, sizeof(ubo));

    auto& modelShader = resourceManager->getShader("Model_Shader");

    while(!glfwWindowShouldClose(window_)) {

        this->showFPS();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearDepth(1.0);

        Camera_.updateMatrices();
        ubo.view = Camera_.getView();
        ubo.proj = Camera_.getProj();
        ubo.model = glm::mat4(1.0f);
        resourceManager->updateBuffer("Matrices", (const unsigned char*)&ubo, sizeof(ubo));

        modelShader.use();

        modelShader.setFloat("time", lastFrame_);
        modelShader.setVec3("cameraWorldPos", Camera_.getPosition());

        for (auto& model : Models_) {
            model.draw(modelShader);
        }

        sceneManager->drawEnvironment();

        glfwSwapBuffers(window_);
        glfwPollEvents();    
    }
}


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
}


void PotentialApp::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(MouseHidden_) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            Camera_.moveFront(4 * deltaTime_);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            Camera_.moveFront(-4 * deltaTime_);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            Camera_.moveRight(-4 * deltaTime_);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            Camera_.moveRight(4 * deltaTime_);
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
    }

}


PotentialApp::PotentialApp() {
    m_Application = this;

    enableDefaultMSAA_ = false;// true;
    samples_ = 1;
}


void PotentialApp::initRender() {
    auto sceneManager = SceneResources::SceneManager::getInstance();
    auto resourceManager = Resources::ResourceManager::getInstance();

    Resources::ShaderDesc shaderDesc = {"Model_Shader", "../shaders/Model.vert", "../shaders/Model.frag"};
    auto& modelShader = resourceManager->createShader(shaderDesc);


    
    const std::vector<std::string> texturesNames = {
        "../textures/posx.jpg",
        "../textures/negx.jpg",
        "../textures/posy.jpg",
        "../textures/negy.jpg",
        "../textures/posz.jpg",
        "../textures/negz.jpg"
    };
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::SKYBOX, texturesNames);
    
    /*
    const std::vector<std::string> texturesNames = {
        "../textures/city.jpg"
    };
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::BACKGROUND_IMAGE_2D, texturesNames);
    */
    /*
    const std::vector<std::string> texturesNames = {
        "../textures/equirect.jpg"
    };
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::EQUIRECTANGULAR, texturesNames);
    */

    Resources::BufferDesc bufDesc;
    bufDesc.name = "Matrices";
    bufDesc.bytesize = sizeof(Matrices);
    bufDesc.target = GL_UNIFORM_BUFFER;
    bufDesc.p_data = nullptr;

    resourceManager->createBuffer(bufDesc);

    auto& envShader = resourceManager->getShader("Default_Environment");

    resourceManager->bindBufferShader("Matrices", 0, modelShader);
    resourceManager->bindBufferShader("Matrices", 0, envShader);


    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW);
}


void PotentialApp::initCamera() {
    Camera_.setAspect((float)windowWidth_ / windowHeight_);
    Camera_.setZNear(0.01f);
    Camera_.setZFar(100.0f);
}
