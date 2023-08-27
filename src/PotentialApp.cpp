#include "../headers/PotentialApp.hpp"
#include "../headers/ResourceManager.hpp"
#include "../headers/SceneManager.hpp"

void PotentialApp::renderToWindow() {
    
    this->initRender();

    this->initCamera();

    GLTF::GLTFLoader* GLTFloader_ = GLTF::GLTFLoader::getInstance();
    GLTFloader_->load(Model_.getModelRef(), Model_.getFilename());
    Model_.init();

    struct Matrices {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 model;
    } ubo;

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();

    resourceManager->createUBO<Matrices>(0, &ubo, sizeof(ubo), "Matrices");
    ModelShader_.bindUBO(0, "Matrices");

    while(!glfwWindowShouldClose(window_)) {

        this->showFPS();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        Camera_.updateMatrices();
        ubo.view = Camera_.getView();
        ubo.proj = Camera_.getProj();
        ubo.model = glm::mat4(1.0f);
        resourceManager->updateUBO(&ubo, sizeof(ubo), 0, "Matrices");

        ModelShader_.use();

        ModelShader_.setFloat("time", lastFrame_);
        ModelShader_.setVec3("cameraWorldPos", Camera_.getPosition());

        Model_.draw(ModelShader_);
        sceneManager->drawEnvironment();

        glfwSwapBuffers(window_);
        glfwPollEvents();    
    }

    glDeleteVertexArrays(VAOs_.size(), &VAOs_[0]);
    glDeleteBuffers(VBOs_.size(), &VBOs_[0]);
    VAOs_.resize(0);
    VBOs_.resize(0);
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

    enableDefaultMSAA_ = true;
    samples_ = 4;
}


void PotentialApp::initRender() {
    auto sceneManager = SceneResources::SceneManager::getInstance();

    /*
    const std::vector<std::string> texturesNames = {
        "../textures/posx.jpg",
        "../textures/negx.jpg",
        "../textures/posy.jpg",
        "../textures/negy.jpg",
        "../textures/posz.jpg",
        "../textures/negz.jpg"
    };
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::SKYBOX, texturesNames);
    */

    const std::vector<std::string> texturesNames = {
        "../textures/city.jpg"
    };
    sceneManager->createEnvironment(SceneResources::SceneManager::EnvironmentType::BACKGROUND_IMAGE_2D, texturesNames);

    ModelShader_ = GeneralApp::Shader("../shaders/Model.vert", "../shaders/Model.frag");

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
