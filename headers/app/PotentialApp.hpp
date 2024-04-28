#ifndef POTENTIALAPP_HPP
#define POTENTIALAPP_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

#define GLM_FORCE_SWIZZLE
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

static inline constexpr const char* CONFIG_PATH = "configs://models.json";
static inline constexpr const char* MODEL_SHADER_NAME = "Model_Shader";
static inline constexpr const char* MODEL_FRAMEBUFFER_NAME = "MODEL_FRAMEBUFFER";
static inline constexpr const char* MODEL_FRAMEBUFFER_TEXTURE_NAME = "MODEL_FRAMEBUFFER_TEXTURE";

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
    bool ReadyForRender_ = false;
    bool NeedInit_ = false;
    Resources::ResourceHandle previewTextureHandle_;
    uint64_t frameAfterInit_ = 0;


    std::vector<Geometry::Model> Models_;

    Resources::ResourceHandle modelShaderHandle_;
    Resources::ResourceHandle modelFramebufferHandle_;
    Resources::ResourceHandle modelFramebufferTextureHandle_;

    bool keyPressedA_ = false;
    bool keyPressedW_ = false;
    bool keyPressedS_ = false;
    bool keyPressedD_ = false;

    void processMovement();

public:
    PotentialApp();

    void OnInit() override;
    void OnWindowCreate() override;
    void OnRenderingStart() override;
    void OnRenderFrame() override;
    void OnRenderingEnd() override;
    void OnWindowDestroy() override;

#ifdef __ANDROID__
    int32_t handleInputCallback(android_app* app, AInputEvent* event) override;
#else
    void mouseCallback(GLFWwindow* window, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;
    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) override;
    void framebufferSizeCallback(GLFWwindow* window, int width, int height) override;
    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
#endif

    void initModels();
    void initLights();
    void initRender();
    void initCamera();
};


#endif