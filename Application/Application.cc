#include "Application.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static struct _GLFW {
    _GLFW() {
        glfwSetErrorCallback([](int error, const char* description) {
            fprintf(stderr, "GLFW Error %d: %s\n", error, description);
        });
        if (!glfwInit()) abort();

    }
    ~_GLFW() {
        glfwTerminate();
    }
} _glfw;

Application::Application(const char * title, int width, int height) {
    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    // Create window with graphics context
    if (!title) title = "OpenGL Application";
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) abort();

    bindContext();
    glfwSwapInterval(1);

    // Initialize OpenGL loader
    int err = gl3wInit();
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(1);
    }

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    //io.IniFilename = NULL; // Disable saving to file "imgui.ini"

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup style
    //ImGui::StyleColorsLight();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    unbindContext();
    glfwDestroyWindow(window);
}

void Application::newFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void Application::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    unbindContext(); // allow other thread get context during swap waiting
    glfwSwapBuffers(window);
    bindContext();
}

bool Application::shouldClose() { return glfwWindowShouldClose(window); }
void Application::close() { glfwSetWindowShouldClose(window, true); }

void Application::bindContext() {
    mutex.lock();
    glfwMakeContextCurrent(window);
}
void Application::unbindContext() {
    glfwMakeContextCurrent(nullptr);
    mutex.unlock();
}

Application::ContextRAII Application::getScopedContext() {
    if (glfwGetCurrentContext() == window)
        return ContextRAII(nullptr);
    return ContextRAII(this);
}
