#pragma once

#include <GL/gl3w.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>

#include "imgui.h"

class Application {
protected:
    int fps_limit;
    double next_time;
    struct GLFWwindow * window = nullptr;
    std::mutex mutex;
    std::condition_variable cv;
    bool binded = false;
    std::queue<std::thread::id> waiting_threads;
    Application(const char * title = nullptr, int width = 960, int height = 720, int fps = 60);
    ~Application();
    void newFrame();
    void endFrame();
    // thread safe (exclusive)
    void bindContext();
    void unbindContext();
public:
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
