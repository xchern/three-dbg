#pragma once

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <map>

#include "drawer.h"
#include "camera.h"

class ViewerApp {
public:
    GLFWwindow * window;
    bool cc = false;
    ViewerApp(int width = 1280, int height = 720);
    ~ViewerApp();
    Camera cam;
    struct DrawerItem{
        bool enable;
        std::unique_ptr<Drawer> ptr;
    };
    std::map<std::string, struct DrawerItem> drawers;
    void addDrawer(std::string name, std::unique_ptr<Drawer> && d) {
        if (drawers.find(name) != drawers.end())
            drawers[name].ptr = std::move(d);
        else
            drawers[name] = {true, std::move(d)};
    }
    void ShowDrawers();
    void closeWindow() {
        glfwSetWindowShouldClose(window, true);
    }
    int loopOnce();
};
