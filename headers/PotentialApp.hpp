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

#include "IApplication.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Cube.hpp"
#include "GLTFLoader.hpp"
#include "Model.hpp"
#include "Mesh.hpp"

#include <tinygltf/tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class PotentialApp : public GeneralApp::IApplication {
private:
    struct Matrices {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 model;
    };

    GeneralApp::Camera Camera_{glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};

    float MouseLastX_ = windowWidth_ / 2.0f, MouseLastY_ = windowHeight_ / 2.0f;
    bool FirstMouse_ = true;
    bool IsFullscreen_ = false;
    bool IsWireframe_ = false;

    bool MouseHidden_ = true;

    std::vector<Geometry::Model> Models_;

public:
    PotentialApp();

    void renderToWindow() override;

    void mouseCallback(GLFWwindow* window, int button, int action, int mods) override;

    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;

    void framebufferSizeCallback(GLFWwindow* window, int width, int height) override;

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;

    void initModels();

    void initRender();

    void initCamera();
};


#endif