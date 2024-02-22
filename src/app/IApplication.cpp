#include "IApplication.hpp"
#include "Logger.hpp"


namespace GeneralApp {

IApplication* IApplication::m_Application;

IApplication::~IApplication() {
    IApplication::terminateWindow();
}


GLFWwindow* IApplication::createWindow(const unsigned& GLFWVerMaj, const unsigned& GLFWVerMin, const unsigned& winWidth,
    const unsigned& winHeight, const std::string& name) {

    LOG_I("Creating the window...");
    OnInit();

    windowName_ = name;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFWVerMaj);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFWVerMin);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    if(enableDefaultMSAA_) {
        glfwWindowHint(GLFW_SAMPLES, samples_);
    }

    window_ = glfwCreateWindow(winWidth, winHeight, name.c_str(), NULL, NULL);
    windowWidth_  = winWidth;
    windowHeight_ = winHeight;
    if (!window_) {
        LOG_E("Failed to create window");
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window_);
    
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallbackStatic);
    glfwSetMouseButtonCallback(window_, mouseCallbackStatic);
    glfwSetCursorPosCallback(window_, cursorCallbackStatic);
    glfwSetScrollCallback(window_, scrollCallbackStatic);
    glfwSetKeyCallback(window_, keyboardCallbackStatic);

    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGL();

    if(enableDefaultMSAA_) {
        glEnable(GL_MULTISAMPLE);
    }

    OnWindowCreate();

    return window_;
}

void IApplication::renderToWindow() {
    OnRenderingStart();
    while (!glfwWindowShouldClose(window_)) {

        if (needToRender_) {
            OnRenderFrame();
            glfwSwapBuffers(window_);
        }
        glfwPollEvents();
    }
    OnRenderingEnd();
}


void IApplication::showFPS() {
    float currentFrame = glfwGetTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;
    ++nbFrames_;

    if (currentFrame - lastFrameFPS_ >= 1.0f) {
        std::stringstream ss;
        ss << windowName_ + " [" << static_cast<int>(static_cast<float>(nbFrames_) / (currentFrame - lastFrameFPS_)) << " FPS ]";
        glfwSetWindowTitle(window_, ss.str().c_str());

        lastFrameFPS_ = currentFrame;
        nbFrames_ = 0;
    }
}


void IApplication::terminateWindow() {
    if (!isTerminated_) {
        isTerminated_ = true;
        LOG_I("Terminating the window...");
        glfwTerminate();
        OnWindowDestroy();
    }
}

}