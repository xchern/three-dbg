#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "ViewerApp.h"

#include <thread>
#include <mutex>

namespace threedbg {
static std::thread displayThread;
static std::mutex lock;
static std::map<std::string, std::unique_ptr<DrawerFactory>> drawerFactories;
static std::unique_ptr<ViewerApp> app = nullptr;
static bool working_flag;

void initDisplay(void) {
    glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "Glfw Error %d: %s\n", error, description);
            });
    if (!glfwInit()) exit(1);
    working_flag = true;
    // new thread for opengl display
    displayThread = std::thread([&](void) {
            app = std::make_unique<ViewerApp>();
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
}
void freeDisplay(void) {
    if (app) app->closeWindow();
    displayThread.join();
    glfwTerminate();
}
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df) {
    lock.lock();
    drawerFactories[name] = std::move(df);
    lock.unlock();
}
bool working(void) {
    return working_flag;
}
}
