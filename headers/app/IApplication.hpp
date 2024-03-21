#ifndef IAPPLICATION_HPP
#define IAPPLICATION_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <android_native_app_glue.h>
#include <jni.h>
#else
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif


namespace GeneralApp {

struct WindowCreateInfo {
#ifdef __ANDROID__
    android_app* app;
#else
    unsigned winWidth;
    unsigned winHeight;
#endif
    std::string name;
};

class IApplication {

protected:

    unsigned windowWidth_ = 0;
    unsigned windowHeight_ = 0;

    std::string windowName_;

    bool isTerminated_ = false;

    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    float lastFrameFPS_ = 0.0f;
    int nbFrames_ = 0;

    bool enableDefaultMSAA_ = false;
    int samples_;

    bool needToRender_ = true;

    static IApplication* m_Application;
    
    void showFPS();

    virtual void OnInit() = 0;
    virtual void OnWindowCreate() = 0;
    virtual void OnRenderingStart() = 0;
    virtual void OnRenderFrame()  = 0;
    virtual void OnRenderingEnd() = 0;
    virtual void OnWindowDestroy() = 0;

#ifdef __ANDROID__
    android_app* android_app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLConfig config_;
    virtual void handleCmdCallback(android_app* app, int32_t cmd);
    virtual int32_t handleInputCallback(android_app* app, AInputEvent* event) = 0;
#else
    GLFWwindow* window_;
    virtual void framebufferSizeCallback(GLFWwindow* window, int width, int height) = 0;
    virtual void cursorCallback(GLFWwindow* window, double xpos, double ypos) = 0;
    virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) = 0;
    virtual void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) = 0;
    virtual void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
#endif

    IApplication() {};
    IApplication(const IApplication& other) {};
    IApplication& operator=(const IApplication& other) { return *this; }
    IApplication(IApplication&& other) noexcept {}
    IApplication& operator=(IApplication&& other) noexcept { return *this; }


private:

#ifdef __ANDROID__
    static void handleCmdCallbackStatic(android_app* app, int32_t cmd) {
        m_Application->handleCmdCallback(app, cmd);
    }

    static int32_t handleInputCallbackStatic(android_app* app, AInputEvent* event) {
        return m_Application->handleInputCallback(app, event);
    }
#else
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
#endif

public:

    virtual ~IApplication();

    void createWindow(const WindowCreateInfo& winInfo);
    void renderToWindow();
    void terminateWindow();
};

}

#endif