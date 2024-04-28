#include "IApplication.hpp"
#include "Logger.hpp"


namespace GeneralApp {

IApplication* IApplication::m_Application;

IApplication::~IApplication() {
    IApplication::terminateWindow();
}


void IApplication::createWindow(const WindowCreateInfo& winInfo) {

    LOG_I("Creating the window...");
    OnInit();

    windowName_ = winInfo.name;

#ifdef __ANDROID__
    android_app_ = winInfo.app;

    android_app_->onAppCmd = handleCmdCallbackStatic;
    android_app_->onInputEvent = handleInputCallbackStatic;

    while (!android_app_->window) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(0, nullptr, &events,(void**)&source)) >= 0) {
            if (source != nullptr) {
                source->process(android_app_, source);
            }
        }
    }
#else
    windowWidth_ = winInfo.winWidth;
    windowHeight_ = winInfo.winHeight;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if(enableDefaultMSAA_) {
        glfwWindowHint(GLFW_SAMPLES, samples_);
    }

    window_ = glfwCreateWindow(windowWidth_, windowHeight_, windowName_.c_str(), NULL, NULL);
    if (!window_) {
        LOG_E("Failed to create window");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(0);

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
#endif

    OnWindowCreate();

    return;
}

void IApplication::renderToWindow() {

    OnRenderingStart();

#ifdef __ANDROID__

    bool run = true;

    while (run) {
        int ident;
        int events;
        struct android_poll_source* source;

        if (needToRender_) {
            OnRenderFrame();
            eglSwapBuffers(display_, surface_);
        }

        while ((ident = ALooper_pollAll(0, nullptr, &events,(void**)&source)) >= 0) {
            if (source != nullptr) {
                source->process(android_app_, source);
            }

            if (android_app_->destroyRequested != 0) {
                terminateWindow();
                run = false;
                break;
            }
        }
    }
#else
    while (!glfwWindowShouldClose(window_)) {

        if (needToRender_) {
            OnRenderFrame();
            glfwSwapBuffers(window_);
        }
        glfwPollEvents();
    }
#endif

    OnRenderingEnd();
}


void IApplication::showFPS() {
#ifndef __ANDROID__
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
#endif
}


void IApplication::terminateWindow() {
    if (!isTerminated_) {
        isTerminated_ = true;
        LOG_I("Terminating the window...");

#ifdef __ANDROID__
        if (display_ != EGL_NO_DISPLAY) {
            eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (surface_ != EGL_NO_SURFACE) {
                eglDestroySurface(display_, surface_);
            }
        }
        surface_ = EGL_NO_SURFACE;
#else
        glfwTerminate();
#endif

        OnWindowDestroy();
    }
}

#ifdef __ANDROID__
void IApplication::handleCmdCallback(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            LOG_I("APP_CMD_INIT_WINDOW()");
            if (app->window != nullptr) {
                android_app_->window = app->window;

                if (isTerminated_) {
                    // If the windows was terminated and we get back to the app create new window and surface
                    surface_ = eglCreateWindowSurface(display_, config_, android_app_->window, nullptr);
                    if (!surface_) {
                        LOG_E("Failed to create render surface");
                        break;
                    }
                    isTerminated_ = false;
                }
                else {
                    // If it is our first window and surface just create them and OpenGLES context as well
                    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                              EGL_BLUE_SIZE, 8,
                                              EGL_GREEN_SIZE, 8,
                                              EGL_RED_SIZE, 8,
                                              EGL_NONE};

                    EGLint numConfigs;

                    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
                    eglInitialize(display_, nullptr, nullptr);

                    eglChooseConfig(display_, attribs, nullptr, 0, &numConfigs);
                    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
                    eglChooseConfig(display_, attribs, supportedConfigs.get(), numConfigs,
                                    &numConfigs);

                    EGLint i = 0;
                    for (; i < numConfigs; ++i) {
                        auto &cfg = supportedConfigs[i];
                        EGLint r, g, b, d;
                        if (eglGetConfigAttrib(display_, cfg, EGL_RED_SIZE, &r) &&
                            eglGetConfigAttrib(display_, cfg, EGL_GREEN_SIZE, &g) &&
                            eglGetConfigAttrib(display_, cfg, EGL_BLUE_SIZE, &b) &&
                            eglGetConfigAttrib(display_, cfg, EGL_DEPTH_SIZE, &d) &&
                            r == 8 && g == 8 && b == 8 && d == 0) {
                            config_ = supportedConfigs[i];
                            break;
                        }
                    }
                    if (i == numConfigs) {
                        config_ = supportedConfigs[0];
                    }

                    if (config_ == nullptr) {
                        LOG_E("Failed to initialize EGLConfig");
                        break;
                    }

                    eglBindAPI(EGL_OPENGL_ES_API);
                    const EGLint ctx_attribs[] = {
                            EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE
                    };

                    surface_ = eglCreateWindowSurface(display_, config_, android_app_->window,
                                                      nullptr);
                    if (!surface_) {
                        LOG_E("Failed to create render surface");
                        break;
                    }

                    context_ = eglCreateContext(display_, config_, nullptr, ctx_attribs);
                    if (!context_) {
                        LOG_E("Failed to create OpenGL ES context");
                        break;
                    }

                    LOG_I("Created OpenGL ES context %p", context_);
                }

                if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
                    LOG_E("Failed to eglMakeCurrent");
                    break;
                }

                windowWidth_ = ANativeWindow_getWidth(android_app_->window);
                windowHeight_ = ANativeWindow_getHeight(android_app_->window);

                OnWindowCreate();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            LOG_I("APP_CMD_TERM_WINDOW()");
            terminateWindow();
            break;
        case APP_CMD_DESTROY:
            LOG_I("APP_CMD_DESTROY()");
            terminateWindow();

            // Manually destroy OpenGLES context and display
            if (display_ != EGL_NO_DISPLAY && context_ != EGL_NO_CONTEXT) {
                eglDestroyContext(display_, context_);
            }
            eglTerminate(display_);
            context_ = EGL_NO_CONTEXT;
            display_ = EGL_NO_DISPLAY;
            break;
        default:
            break;
    }
}
#endif

}