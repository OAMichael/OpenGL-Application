#ifndef POTENTIALAPP_HPP
#define POTENTIALAPP_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../headers/IApplication.hpp"
#include "../headers/Shader.hpp"
#include "../headers/Camera.hpp"
#include "../headers/Cube.hpp"
#include "../headers/GLTFLoader.hpp"
#include "../headers/Model.hpp"
#include "../headers/Mesh.hpp"

#include <tinygltf/tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class PotentialApp : public GeneralApp::IApplication {
private:
    
    std::vector<unsigned> VBOs_;
    std::vector<unsigned> VAOs_; 
    std::vector<unsigned> EBOs_;

    GeneralApp::Camera Camera_{glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};

    float MouseLastX_ = windowWidth_ / 2.0f, MouseLastY_ = windowHeight_ / 2.0f;
    bool FirstMouse_ = true;
    bool IsFullscreen_ = false;

    GeneralApp::Shader ModelShader_;

    bool MouseHidden_ = true;

    Geometry::Model Model_{"../models/AntiqueCamera/AntiqueCamera.gltf"};

public:
    PotentialApp();

    void renderToWindow() override;

    void mouseCallback(GLFWwindow* window, int button, int action, int mods) override;

    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

    void framebufferSizeCallback(GLFWwindow* window, int width, int height) override;

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void initRender();

    void initCamera();
};


#endif