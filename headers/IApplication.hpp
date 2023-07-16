#ifndef IAPPLICATION_HPP
#define IAPPLICATION_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace GeneralApp {

class IApplication {

protected:

    GLFWwindow* window_;
    unsigned windowWidth_;
    unsigned windowHeight_;

    std::string windowName_;

    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    float lastFrameFPS_ = 0.0f;
    int nbFrames_ = 0;

    bool enableDefaultMSAA_ = false;
    int samples_;

    static IApplication* m_Application;
    
    virtual void renderToWindow() = 0;
    virtual void framebufferSizeCallback(GLFWwindow* window, int width, int height) = 0;
    virtual void cursorCallback(GLFWwindow* window, double xpos, double ypos) = 0;
    virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) = 0;
    virtual void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) = 0;
    virtual void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;

    void showFPS();

    IApplication() {};
    IApplication(const IApplication& other) {};
    IApplication& operator=(const IApplication& other) { return *this; }
    IApplication(IApplication&& other) noexcept {}
    IApplication& operator=(IApplication&& other) noexcept { return *this; }


private:

    static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
        m_Application->framebufferSizeCallback(window, width, height);
    }

    static void cursorCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
        m_Application->cursorCallback(window, xpos, ypos);
    }

    static void mouseCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        m_Application->mouseCallback(window, button, action, mods);
    }

    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
        m_Application->scrollCallback(window, xoffset, yoffset);
    }

    static void keyboardCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        m_Application->keyboardCallback(window, key, scancode, action, mods);
    }

public:

    virtual ~IApplication();

    GLFWwindow* createWindow(const unsigned& GLFWVerMaj, const unsigned& GLFWVerMin, const unsigned& winWidth,
        const unsigned& winHeight, const std::string& name);

    void terminateWindow();
};

}

#endif