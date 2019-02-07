#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "ViewerApp.h"

/* #define MULTI_THREAD */

#ifdef MULTI_THREAD
#include <thread>
#include <mutex>
#endif

namespace threedbg {
#ifdef MULTI_THREAD
static std::thread displayThread;
static std::mutex lock;
#endif
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

void initDisplay(void) {
    glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "Glfw Error %d: %s\n", error, description);
            });
    if (!glfwInit()) exit(1);
    // new thread for opengl display
#ifdef MULTI_THREAD
    displayThread = std::thread([&](void) {
#endif
            app = std::make_unique<ViewerApp>();
            working_flag = true;
#ifdef MULTI_THREAD
            while (!app->loopOnce()) {
            lock.lock();
            std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
            drawerFactories.clear();
            lock.unlock();
            for (auto & p : dfs)
            app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
            }
            app.reset(nullptr);
            working_flag = false;
            });
    while(!working()) {}
#endif
}
void freeDisplay(void) {
    if (app) app->closeWindow();
#ifdef MULTI_THREAD
    displayThread.join();
#endif
    glfwTerminate();
}
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df) {
#ifdef MULTI_THREAD
    lock.lock();
#endif
    drawerFactories[name] = std::move(df);
#ifdef MULTI_THREAD
    lock.unlock();
#endif
}
bool working(void) {
#ifndef MULTI_THREAD
    if (!app->loopOnce()) {
        std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
        drawerFactories.clear();
        for (auto & p : dfs)
            app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
    } else {
        app.reset(nullptr);
        working_flag = false;
    }
#endif
    return working_flag;
}
}
