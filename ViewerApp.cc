#include "ViewerApp.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "lines.h"
#include "points.h"

ViewerApp::ViewerApp(int width, int height) {
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
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    // Create window with graphics context
    const char title[] = "3d debug";
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) abort();
    glfwMakeContextCurrent(window);
    /* glfwSwapInterval(1); */

    // Initialize OpenGL loader
    int err = gl3wInit();
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(1);
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9,0.9,0.9,1);

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.IniFilename = NULL; // Disable saving to file "imgui.ini"

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup style
    //ImGui::StyleColorsLight();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    PointsDrawer::initGL();
    LinesDrawer::initGL();
}
ViewerApp::~ViewerApp() {
    LinesDrawer::freeGL();
    PointsDrawer::freeGL();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}
void ViewerApp::ShowDrawers() {
    if (ImGui::CollapsingHeader("Drawers")) {
        for (auto & d : drawers)
            ImGui::Checkbox(d.first.c_str(), &d.second.enable);
    }
}
int ViewerApp::loopOnce() {
    if (glfwWindowShouldClose(window)) return 1;
    glfwPollEvents();
    struct draw_param dp;
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        dp.resolution[0] = w;
        dp.resolution[1] = h;
        auto mat = cam.getMat(dp.resolution[0]/dp.resolution[1]);
        memcpy(&dp.mat, &mat, 16 * sizeof(float));
        dp.cam = cam;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    const float dist = 8.f;
    ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - dist, dist);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    if (ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        ImGui::Text("framerate: %.1ffps", ImGui::GetIO().Framerate);
        ImGui::Checkbox("control camera", &cc);
    }
    ShowDrawers();
    ImGui::End();
    if (cc) {

        if (ImGui::Begin("camera")) {
            cam.ImGuiDrag();
            cam.ImGuiEdit();
        }
        ImGui::End();
    }

    for (auto & d : drawers)
        if (d.second.enable)
            d.second.ptr->draw(dp);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    return 0;
}
