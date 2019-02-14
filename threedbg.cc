#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "ViewerApp.h"

/* #define MULTI_THREAD */

#include <thread>
#include <mutex>

namespace threedbg {
bool multithread = true;
static std::thread displayThread;
static std::mutex lock;
static std::map<std::string, std::unique_ptr<DrawerFactory>> drawerFactories;
static std::unique_ptr<ViewerApp> app = nullptr;
static bool working_flag;

Camera getCamera() {
    if (app) return app->cam;
    else return Camera();
}
void setCamera(const Camera & c) {
    if (app) app->cam = c;
}

static bool loopOnce() {
    if (!app->loopOnce()) {
        lock.lock();
        std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
        drawerFactories.clear();
        lock.unlock();
        for (auto & p : dfs)
            app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
        return true;
    } else {
        working_flag = false;
        return false;
    }
}

void initDisplay(void) {
    glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "Glfw Error %d: %s\n", error, description);
            });
    if (!glfwInit()) exit(1);
    if (multithread) {
    displayThread = std::thread([&](void) { // new thread for opengl display
            app = std::make_unique<ViewerApp>();
            working_flag = true;
            while(loopOnce());
            app.reset(nullptr);
            });
    while (!working_flag) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
    app = std::make_unique<ViewerApp>();
    working_flag = true;
    }
}
void freeDisplay(void) {
    if (app) app->closeWindow();
    if (multithread)
        displayThread.join();
    else
        app.reset(nullptr);
    glfwTerminate();
}
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df) {
    lock.lock();
    drawerFactories[name] = std::move(df);
    lock.unlock();
}
bool working(void) {
    if (!multithread)
        loopOnce();
    return working_flag;
}
}
