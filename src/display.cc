#include "display.h"

using namespace threedbg;
using namespace threedbg::display;

#include <vector>

#include <GLFW/glfw3.h>

static GLFWwindow *window;
static bool done;
static int w, h;

static enum {NORMAL, ROTATE, GRAB} manipState = NORMAL;
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            if (mods & GLFW_MOD_SHIFT) manipState = GRAB;
            else {
                manipState = ROTATE;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        } else if (action == GLFW_RELEASE) {
            manipState = NORMAL;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
    static double x, y;
    double dx = (xpos - x) / h, dy = (ypos - y) / h;
    x = xpos; y = ypos;
    switch (manipState){
    case ROTATE:
        camera::rotateEye(-dx * camera::getFovy(), dy * camera::getFovy());
        break;
    case GRAB:
        camera::translate(-dx * camera::getDist() * camera::getFovy(),
                          dy * camera::getDist() * camera::getFovy());
        break;
    }
}

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwMakeContextCurrent(window);
    gl3wInit();
    glClearColor(.2, .2, .2, 1);
    glEnable(GL_DEPTH_TEST);
    threedbg::Point::init();
}

bool threedbg::display::finished(void) { return done; }

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