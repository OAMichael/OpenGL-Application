#ifndef POTENTIALAPP_HPP
#define POTENTIALAPP_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../headers/Application.hpp"
#include "../headers/Shader.hpp"
#include "../headers/Camera.hpp"
#include "../headers/Cube.hpp"



class PotentialApp : public GeneralApp::Application {
private:
    GeneralApp::Shader Shader_;
    
    std::vector<unsigned> VBOs_;
    std::vector<unsigned> VAOs_; 
    std::vector<unsigned> EBOs_;

    GeneralApp::Camera Camera_{glm::vec3(0.0f, 4.0f, 6.0f), glm::vec3(0.0f, 4.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};

    float MouseLastX_ = windowWidth_ / 2.0f, MouseLastY_ = windowHeight_ / 2.0f;
    bool FirstMouse_ = true;
    bool IsFullscreen_ = false;

    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    float lastFrameFPS_ = 0.0f;
    int nbFrames_ = 0;

    bool blocksUpdated_ = true;

    std::vector<Geometry::Cube> GrassBlocks_;
    GeneralApp::Shader CubemapShader_;

    Geometry::Cube Skybox_{glm::vec3(0.0f, 0.0f, 0.0f)};
    GeneralApp::Shader SkyboxShader_;

    bool MouseHidden_ = true;

public:
    PotentialApp();

    void renderToWindow() override;

    void mouseCallback(GLFWwindow* window, int button, int action, int mods) override;

    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;

    void framebufferSizeCallback(GLFWwindow* window, int width, int height) override;

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void initRender();

    void initCamera();

    void setVisibleBlocksPositions();

    void showFPS();
};


#endif