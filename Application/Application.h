#pragma once

#include <GL/gl3w.h>

#include <mutex>

#include "imgui.h"

class Application {
protected:
    struct GLFWwindow * window = nullptr;
    Application(const char * title = nullptr, int interval = 1, int width = 960, int height = 720);
    ~Application();
    void newFrame();
    void endFrame();
public:
    void bindContext();
    void unbindContext();
    void show();
    void hide();
    bool shouldClose();
    void close();

    // scoped version for convinience
    struct ContextRAII{
        Application * ptr;
        ContextRAII(Application * app) : ptr(app) { if (ptr) ptr->bindContext(); }
        ContextRAII(ContextRAII&& c) { ptr = c.ptr; c.ptr = nullptr; }
        ~ContextRAII() { if (ptr) ptr->unbindContext(); }
    };
    ContextRAII getScopedContext();
};
