#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace GeneralApp {

class Application {

protected:
    GLFWwindow* window_;
    unsigned windowWidth_;
    unsigned windowHeight_;

    bool enableDefaultMSAA_ = false;
    int samples_;

    static Application* m_Application;
public:

    Application();
    ~Application();

    GLFWwindow* createWindow(const unsigned& GLFWVerMaj, const unsigned& GLFWVerMin, const unsigned& winWidth,
        const unsigned& winHeight, const std::string& name);

    virtual void renderToWindow() = 0;


    virtual void framebufferSizeCallback(GLFWwindow* window, int width, int height) = 0;

    static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
        m_Application->framebufferSizeCallback(window, width, height);
    }


    virtual void cursorCallback(GLFWwindow* window, double xpos, double ypos) = 0;

    static void cursorCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
        m_Application->cursorCallback(window, xpos, ypos);
    }


    virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) = 0;

    static void mouseCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        m_Application->mouseCallback(window, button, action, mods);
    }


    virtual void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;

    static void keyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        m_Application->keyboardCallback(window, key, scancode, action, mods);
    }
    
    void terminateWindow();
};

}

#endif