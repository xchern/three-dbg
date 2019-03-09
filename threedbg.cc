#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "Application.h"

#define errorfln(fmt, ...) fprintf(stderr, fmt"\n", __VA_ARGS__)

class ThreedbgApp : public Application {
public:
    ThreedbgApp(int width = 1280, int height = 720);
    ~ThreedbgApp();
    int loopOnce();
    void addDrawer(std::string name, std::unique_ptr<Drawer> d) {
        if (drawers.find(name) != drawers.end())
            drawers[name].ptr = std::move(d);
        else
            drawers[name] = {true, std::move(d)};
    }
    void snapshot(int & w, int & h, std::vector<unsigned char> & pixels) {
        auto _ctx = Application::getScopedContext();
        draw();
        w = cam.resolution[0]; h = cam.resolution[1];
        pixels.resize(w* h * 3);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        glCheckError();
    }
private:
    DrawingCtx ctx;
    ImageViewer iv;

    struct DrawerItem {
        bool enable;
        std::unique_ptr<Drawer> ptr;
    };
    std::map<std::string, struct DrawerItem> drawers;
    void draw() {
        ctx.bindFB(cam.resolution.x, cam.resolution.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        struct draw_param dp;
        {
            auto mat = cam.getMat();
            memcpy(&dp.mat, &mat, 16 * sizeof(float));
            dp.cam = cam;
        }
        for (auto & d : drawers)
            if (d.second.enable)
                d.second.ptr->draw(dp);
    }
    Camera cam;
    void ImGuiManipulateCamera() {
        cam.ImGuiDrag();
        cam.ImGuiEdit();
    }
    void ImGuiSwitchDrawers() {
        for (auto & d : drawers) ImGui::Checkbox(d.first.c_str(), &d.second.enable);
    }
};

ThreedbgApp::ThreedbgApp(int width, int height) : Application("3D debug", width, height) {
    ImGui::StyleColorsLight();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9,0.9,0.9,0);
    static bool run = false;
    PointsDrawer::initGL();
    LinesDrawer::initGL();
    glCheckError();
}
ThreedbgApp::~ThreedbgApp() {
    LinesDrawer::freeGL();
    PointsDrawer::freeGL();
    glCheckError();
}
int ThreedbgApp::loopOnce() {
    glCheckError();
    if (Application::shouldClose()) return 1;
    Application::newFrame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList * drawList = ImGui::GetOverlayDrawList();
        drawList->PushClipRectFullScreen();
        {
            char text[64];
            sprintf(text, "%.1f fps", ImGui::GetIO().Framerate);
            ImVec2 size = ImGui::CalcTextSize(text);
            drawList->AddText(ImVec2(io.DisplaySize.x - size.x - 4, io.DisplaySize.y - size.y - 4), 0xff000000, text);
        }
        drawList->PopClipRect();
    }

    if (ImGui::Begin("drawers")) {
        ImGuiSwitchDrawers();
    }
    ImGui::End();
    if (ImGui::Begin("camera")) {
        ImGuiManipulateCamera();
    }
    ImGui::End();

    draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ImGui::Begin("image")) {
        iv.Show(ctx.texture, ImVec2(ctx.resolution[0], ctx.resolution[1]));
    }
    ImGui::End();

    //ImGui::ShowDemoWindow();

    Application::endFrame();
    glCheckError();
    return 0;
}

#include <thread>
#include <mutex>

namespace threedbg {
// basicly ThreedbgApp + thread-safe drawerfactories as cache
bool multithread = true;
static std::thread displayThread;
static std::mutex lock; // for drawerFactories
static std::map<std::string, std::unique_ptr<DrawerFactory>> drawerFactories;
static std::unique_ptr<ThreedbgApp> app = nullptr;

static void makeDrawers() {
    lock.lock();
    std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
    drawerFactories.clear();
    lock.unlock();
    {
        auto _ctx = app->getScopedContext();
        for (auto & p : dfs)
            app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
    }
}

static bool loopOnce() {
    makeDrawers();
    return !app->loopOnce();
}

void initDisplay(void) {
    if (multithread) {
        displayThread = std::thread([&](void) { // new thread for opengl display
            app = std::make_unique<ThreedbgApp>();
            while (loopOnce());
            app.reset(nullptr);
        });
        while (!working()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    else {
        app = std::make_unique<ThreedbgApp>();
    }
}
void freeDisplay(bool force) {
    if (force && app) app->close();
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
void snapshot(int & w, int & h, std::vector<unsigned char> & pixels) {
    app->snapshot(w, h, pixels);
}
}
