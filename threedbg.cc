#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "Application.h"

class ThreedbgApp : public Application {
public:
    ThreedbgApp(int width = 1280, int height = 720);
    ~ThreedbgApp();
    // get context before calling
    void addDrawer(std::string name, std::unique_ptr<Drawer> && d) {
        if (drawers.find(name) != drawers.end())
            drawers[name].ptr = std::move(d);
        else
            drawers[name] = {true, std::move(d)};
    }
    int loopOnce();
private:
    bool cc = false;
    Camera cam;
    struct DrawerItem {
        bool enable;
        std::unique_ptr<Drawer> ptr;
    };
    std::map<std::string, struct DrawerItem> drawers;
    void ShowDrawers();
};

ThreedbgApp::ThreedbgApp(int width, int height) : Application("3D debug", width, height) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9,0.9,0.9,1);
    PointsDrawer::initGL();
    LinesDrawer::initGL();
    glCheckError();
}
ThreedbgApp::~ThreedbgApp() {
    drawers.clear();
    LinesDrawer::freeGL();
    PointsDrawer::freeGL();
    glCheckError();
}
void ThreedbgApp::ShowDrawers() {
    if (ImGui::CollapsingHeader("Drawers")) {
        for (auto & d : drawers)
            ImGui::Checkbox(d.first.c_str(), &d.second.enable);
    }
}
int ThreedbgApp::loopOnce() {
    glCheckError();
    if (Application::shouldClose()) return 1;
    Application::newFrame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    { // update camera resolution
        auto &io = ImGui::GetIO();
        cam.resolution[0] = io.DisplaySize.x;
        cam.resolution[1] = io.DisplaySize.y;
    }
    struct draw_param dp;
    {
        auto mat = cam.getMat();
        memcpy(&dp.mat, &mat, 16 * sizeof(float));
        dp.cam = cam;
    }
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
    glCheckError();
    Application::endFrame();
    glCheckError();
    return 0;
}

#include <thread>

namespace threedbg {
// basicly ThreedbgApp + thread-safe drawerfactories as cache
bool multithread = true;
static std::thread displayThread;
static std::mutex lock; // for drawerFactories
static std::map<std::string, std::unique_ptr<DrawerFactory>> drawerFactories;
static std::unique_ptr<ThreedbgApp> app = nullptr;

static bool loopOnce() {
    if (!app->loopOnce()) {
        lock.lock();
        std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
        drawerFactories.clear();
        lock.unlock();
        {
            auto _ctx = app->getScopedContext();
            for (auto & p : dfs)
                app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
        }
        return true;
    } else {
        return false;
    }
}

void initDisplay(void) {
    if (multithread) {
    displayThread = std::thread([&](void) { // new thread for opengl display
            app = std::make_unique<ThreedbgApp>();
            while(loopOnce());
            app.reset(nullptr);
            });
    while (!working()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
    app = std::make_unique<ThreedbgApp>();
    }
}
void freeDisplay(void) {
    if (app) app->close();
    if (multithread)
        displayThread.join();
    else
        app.reset(nullptr);
}
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df) {
    lock.lock();
    drawerFactories[name] = std::move(df);
    lock.unlock();
}
bool working(void) {
    if (!multithread)
        loopOnce();
    return app && !app->shouldClose();
}
}
