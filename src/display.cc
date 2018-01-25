#include "display.h"

using namespace threedbg::display;

#include <vector>

#include <GLFW/glfw3.h>

static GLFWwindow *window;
static bool done;

void threedbg::display::init(void) {
    done = false;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    // glfwWindowHint(GLFW_DOUBLEBUFFER, 0);

    window = glfwCreateWindow(1080, 720, "3D debug", NULL, NULL);
    glfwMakeContextCurrent(window);
    gl3wInit();
    glClearColor(.2, .2, .2, 1);
    glEnable(GL_DEPTH_TEST);
    threedbg::Point::init();
}

bool threedbg::display::finished(void) { return done; }

static int w, h;
void threedbg::display::loopOnce(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

    threedbg::Point::draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
    if (glfwWindowShouldClose(window))
        done = true;
}

float threedbg::display::getAspect(void) { return (float)w / h; }

void threedbg::display::free(void) {
    threedbg::Point::free();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// interaction