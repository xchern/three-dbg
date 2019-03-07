#pragma once

#include <GL/gl3w.h>

#include <mutex>

#include "imgui.h"

class Application {
protected:
    struct GLFWwindow * window;
    std::mutex mutex;
    Application(const char * title = nullptr, int width = 960, int height = 720);
    ~Application();
    void newFrame();
    void endFrame();
public:
    bool shouldClose();
    void close();
    // thread safe (exclusive)
    void bindContext();
    void unbindContext();

    // scoped version for convinience
    struct ContextRAII{
        Application * ptr;
        ContextRAII(Application * app) : ptr(app) { ptr->bindContext(); }
        ContextRAII(ContextRAII&& c) { ptr = c.ptr; c.ptr = nullptr; }
        ~ContextRAII() { if (ptr) ptr->unbindContext(); }
    };
    ContextRAII getScopedContext() {
        return ContextRAII(this);
    }
};
