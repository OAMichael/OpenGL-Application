#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#include "../headers/PotentialApp.hpp"


using iterCube = typename std::vector<Geometry::Cube>::iterator;
using iterVec3 = typename std::vector<glm::ivec3>::iterator;


void PotentialApp::renderToWindow() {
    
    for(int i = 0; i < 10; ++i)
        for(int j = 0; j < 10; ++j)
            GrassBlocks_.push_back(Geometry::Cube(glm::ivec3(i, 0, j)));

    this->initRender();

    this->initCamera();

    while(!glfwWindowShouldClose(window_)) {

        this->showFPS();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        

        //Shader_.use();
        CubemapShader_.use();

        glBindVertexArray(VAOs_[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, GrassBlocks_[0].getCubemapTextureHandle());

        glm::mat4 view  = glm::lookAt(Camera_.getPosition(), Camera_.getTarget(), Camera_.getUp());
        glm::mat4 proj  = glm::perspective(glm::radians(Camera_.getFov()), Camera_.getAspect(), Camera_.getZNear(), Camera_.getZFar());

        CubemapShader_.setMat4("view", view);
        CubemapShader_.setMat4("proj", proj);


        if(blocksUpdated_) {
            this->setVisibleBlocksPositions();
            blocksUpdated_ = false;
        }

        for(iterCube it = GrassBlocks_.begin(), end = GrassBlocks_.end(); it < end; ++it) {
            if(it->getVisibility()) {
                CubemapShader_.setMat4("model", it->getModelMatrix());
                
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        glDepthFunc(GL_LEQUAL);
        glFrontFace(GL_CCW); 

        SkyboxShader_.use();
        glBindVertexArray(VAOs_[1]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox_.getCubemapTextureHandle());

        SkyboxShader_.setMat4("view",  view);
        SkyboxShader_.setMat4("proj",  proj);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
        glFrontFace(GL_CW);

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
            if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                Camera_.setFov(30.0f);
            if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
                Camera_.setFov(90.0f);
            break;

        case GLFW_KEY_LEFT_SHIFT:
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                Camera_.setDirSpeed(2.5f);
                Camera_.setRightSpeed(1.8f);
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
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
    Shader_ = Shader("../shaders/Triangle.vert", "../shaders/Triangle.frag");
    */

    unsigned VAO, VBO;
    VAOs_.push_back(VAO);
    VBOs_.push_back(VBO);

    glGenVertexArrays(1, &VAOs_[0]);
    glGenBuffers(1, &VBOs_[0]);
    glBindVertexArray(VAOs_[0]);

    std::vector<float> verts = GrassBlocks_[0].getVertices();
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

    GrassBlocks_[0].generateTextures(textures);


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


    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


void PotentialApp::initCamera() {
    Camera_.setAspect((float)windowWidth_ / windowHeight_);
    Camera_.setZNear(0.01f);
    Camera_.setZFar(100.0f);
}


void PotentialApp::showFPS() {
    float currentFrame = glfwGetTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;
    ++nbFrames_;


    if(currentFrame - lastFrameFPS_ >= 1.0f) {
        std::stringstream ss;
        ss << "[ " << static_cast<int>(static_cast<float>(nbFrames_) / (currentFrame - lastFrameFPS_)) << " FPS ]";
        glfwSetWindowTitle(window_, ss.str().c_str());

        lastFrameFPS_ = currentFrame;
        nbFrames_ = 0;

    }
}


void PotentialApp::setVisibleBlocksPositions() {    
    iterCube it  = GrassBlocks_.begin();
    iterCube end = GrassBlocks_.end();

    std::vector<glm::ivec3> blocksPositions;

    for(it; it < end; ++it)
        blocksPositions.push_back(it->getPosition());

    
    iterVec3 endPos = blocksPositions.end();
    for(it = GrassBlocks_.begin(); it < end; ++it) {
        Geometry::Cube currBlock = *it;
        glm::ivec3 currPos = currBlock.getPosition();

        iterVec3 it1 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(1, 0, 0));
        iterVec3 it2 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(-1, 0, 0));
        iterVec3 it3 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(0, 1, 0));
        iterVec3 it4 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(0, -1, 0));
        iterVec3 it5 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(0, 0, 1));
        iterVec3 it6 = std::find(blocksPositions.begin(), endPos, currPos + glm::ivec3(0, 0, -1));

        if(it1 != endPos && it2 != endPos && it3 != endPos && it4 != endPos && it5 != endPos && it6 != endPos)
            it->setVisibility(false);
        else
            it->setVisibility(true);
    }
    
}