#include "display.h"

using namespace threedbg;
using namespace threedbg::display;
static std::mutex & lock = threedbg::globalLock;

#include <vector>
#include <queue>
#include <thread>

#include <GLFW/glfw3.h>

static GLFWwindow *window;
static bool doneFlag = true;
static int w, h;
static bool screenShotFlag = false;
static std::vector<glm::fvec3> pixels;
static std::queue<char> charQueue;

static enum { NORMAL, ROTATE, GRAB } manipState = NORMAL;
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
    if (action == GLFW_RELEASE) {
        manipState = NORMAL;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            manipState = ROTATE;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            manipState = GRAB;
            break;
        }
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

static void char_callback(GLFWwindow *window, unsigned int codepoint) {
    lock.lock();
    if (codepoint < 128 && charQueue.size() < 16)
        charQueue.push(codepoint);
    lock.unlock();
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    float dist = camera::getDist();
    dist *= (1 - yoffset / 20);
    camera::setDist(dist);
}

void threedbg::display::init(void) {
    lock.lock();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(1080, 720, "3D debug", NULL, NULL);
    doneFlag = false;
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    gl3wInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(.3, .3, .3, 1);
    threedbg::Point::init();
    threedbg::Line::init();
    lock.unlock();
}

bool threedbg::display::finished(void) { return doneFlag; }

void threedbg::display::loopOnce(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lock.lock();
    glfwGetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);
    lock.unlock();

    threedbg::Point::draw();
    threedbg::Line::draw();
    if (screenShotFlag) {
        glFinish();
        pixels.resize(w * h);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, &pixels[0]);
        screenShotFlag = false;
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
    if (glfwWindowShouldClose(window)) {
        w = h = 0;
        doneFlag = true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
}

float threedbg::display::getAspect(void) { return (float)w / h; }

void threedbg::display::free(void) {
    threedbg::Point::free();
    threedbg::Line::free();
    glfwDestroyWindow(window);
    glfwTerminate();
    doneFlag = true;
}

void threedbg::display::setDisplaySize(int w, int h) {
    if (!finished())
        glfwSetWindowSize(window, w, h);
}

void threedbg::display::getDisplaySize(int *wp, int *hp) {
    lock.lock();
    *wp = w;
    *hp = h;
    lock.unlock();
}

std::vector<glm::fvec3> threedbg::display::getImage(void) {
    // FIXME: In the image, white spot on the particles, while it looks good with display window
    // FIXED: ALPHA channel seems strange while rgb remains well the white is the backgound color
    // set Flag and wait
    if (finished())
        return std::vector<glm::fvec3>();
    screenShotFlag = true;
    while (screenShotFlag)
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    // move data
    std::vector<glm::fvec3> data = std::move(pixels);
    pixels.clear();
    return data;
}

char threedbg::display::getChar(void) {
    char c = 0;
    lock.lock();
    if (!charQueue.empty()) {
        c = charQueue.front();
        charQueue.pop();
    }
    lock.unlock();
    return c;
}