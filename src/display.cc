#include "display.h"

#include "Point.h"

using namespace threedbg::display;

#include <vector>

static GLFWwindow *window;
static bool done;

void threedbg::display::init(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    // glfwWindowHint(GLFW_DOUBLEBUFFER, 0);

    window = glfwCreateWindow(800, 600, "3D debug", NULL, NULL);
    glfwMakeContextCurrent(window);
    gl3wInit();
    done = false;
    glClearColor(.3, .3, .3, 1);
    threedbg::Point::init();
}

bool threedbg::display::finished(void) { return done; }

void threedbg::display::loopOnce(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

    threedbg::Point::draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
    if (glfwWindowShouldClose(window)) done = true;
}

void threedbg::display::free(void) {
    threedbg::Point::free();
    glfwDestroyWindow(window);
    glfwTerminate();
}

glm::fmat4 threedbg::display::projMat(void) {
    return glm::fmat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}
