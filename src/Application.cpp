#include "../headers/Application.hpp"


namespace GeneralApp {

Application* Application::m_Application;

Application::Application() {

};


Application::~Application() {
    Application::terminateWindow();
};


GLFWwindow* Application::createWindow(const unsigned& GLFWVerMaj, const unsigned& GLFWVerMin, const unsigned& winWidth,
    const unsigned& winHeight, const std::string& name) {

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
    if (!window_)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window_);
    
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallbackStatic);
    glfwSetMouseButtonCallback(window_, mouseCallbackStatic);
    glfwSetCursorPosCallback(window_, cursorCallbackStatic);
    glfwSetKeyCallback(window_, keyboardCallbackStatic);

    //glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGL();

    if(enableDefaultMSAA_) {
        glEnable(GL_MULTISAMPLE);
    }

    return window_;
};


void Application::terminateWindow() {
    glfwTerminate();
};

}