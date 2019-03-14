#include "Application.h"

class Example : public Application {
public:
    Example() {
        ImGui::StyleColorsLight();
    }
    void run() {
        Application::show();
        while (!Application::shouldClose()) {
            Application::newFrame();
            const ImVec4 bg_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            glClearColor(bg_color.x, bg_color.y, bg_color.z, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui::ShowDemoWindow();
            Application::endFrame();
        }
    }
};

int main() {
    Example().run();
}