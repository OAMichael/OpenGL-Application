#include "../headers/PotentialApp.hpp"


void PotentialApp::renderToWindow() {
    
    this->initRender();

    this->initCamera();

    GLTF::GLTFLoader* GLTFloader_ = GLTF::GLTFLoader::getInstance();
    GLTFloader_->load(Model_.getModelRef(), Model_.getFilename());
    Model_.init();

    while(!glfwWindowShouldClose(window_)) {

        this->showFPS();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


        glm::mat4 view  = glm::lookAt(Camera_.getPosition(), Camera_.getPosition() + Camera_.getDirection(), Camera_.getUp());
        glm::mat4 proj  = glm::perspective(glm::radians(Camera_.getFov()), Camera_.getAspect(), Camera_.getZNear(), Camera_.getZFar());
        glm::mat4 model = glm::mat4(1.0f);
        
        ModelShader_.use();
        ModelShader_.setMat4("view", view);
        ModelShader_.setMat4("proj", proj);
        ModelShader_.setMat4("model", model);

        ModelShader_.setFloat("time", lastFrame_);

        Model_.draw(ModelShader_);


        // Skybox shader runs
        glDepthFunc(GL_LEQUAL);
        glFrontFace(GL_CCW);

        SkyboxShader_.use();
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAOs_[1]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox_.getCubemapTextureHandle());

        SkyboxShader_.setMat4("view",  view);
        SkyboxShader_.setMat4("proj",  proj);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
        
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

    /*
    unsigned VAO, VBO;
    VAOs_.push_back(VAO);
    VBOs_.push_back(VBO);

    glGenVertexArrays(1, &VAOs_[0]);
    glGenBuffers(1, &VBOs_[0]);
    glBindVertexArray(VAOs_[0]);

    std::vector<float> verts = Cube_.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, VBOs_[0]);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    

    CubemapShader_ = GeneralApp::Shader("../shaders/Cubemap.vert", "../shaders/Cubemap.frag");

    const std::vector<std::string> textures = {
        "../textures/cobble.jpg",
        "../textures/cobble.jpg",
        "../textures/grass.jpg",
        "../textures/cobble.jpg",
        "../textures/cobble.jpg",
        "../textures/cobble.jpg"
    };

    Cube_.generateTextures(textures);
    */

    unsigned VAOSkybox, VBOSkybox;
    VAOs_.push_back(VAOSkybox);
    VBOs_.push_back(VBOSkybox);

    glGenVertexArrays(1, &VAOs_[1]);
    glGenBuffers(1, &VBOs_[1]);
    glBindVertexArray(VAOs_[1]);

    std::vector<float> vertsSkybox = Skybox_.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, VBOs_[1]);
    glBufferData(GL_ARRAY_BUFFER, vertsSkybox.size() * sizeof(float), vertsSkybox.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    SkyboxShader_ = GeneralApp::Shader("../shaders/Skybox.vert", "../shaders/Skybox.frag");

    const std::vector<std::string> texturesSkybox = {
        "../textures/posx.jpg",
        "../textures/negx.jpg",
        "../textures/posy.jpg",
        "../textures/negy.jpg",
        "../textures/posz.jpg",
        "../textures/negz.jpg"
    };

    Skybox_.generateTextures(texturesSkybox);

    
    ModelShader_ = GeneralApp::Shader("../shaders/Model.vert", "../shaders/Model.frag");


    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


void PotentialApp::initCamera() {
    Camera_.setAspect((float)windowWidth_ / windowHeight_);
    Camera_.setZNear(0.01f);
    Camera_.setZFar(100.0f);
}
