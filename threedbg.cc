#include "threedbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "Application.h"

#define errorfln(fmt, ...) fprintf(stderr, fmt"\n", __VA_ARGS__)

class Blocker {
    int state;
    bool waiting = false;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<float> time_count;
    bool previous_timepoint_set = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
public:
    enum { RUNNING, PAUSED, STEP };
    Blocker() : state(RUNNING) {}
    ~Blocker() {
        std::unique_lock<std::mutex> lk(mtx);
        state = RUNNING;
        cv.notify_all();
        while (waiting) cv.wait(lk);
    }
    void barrier() {
        if (previous_timepoint_set) {
            float ns = (std::chrono::high_resolution_clock::now() - time_point).count() * 1e-6f;
            time_count.push_back(ns);
        }
        {
            std::unique_lock<std::mutex> lk(mtx);
            waiting = true;
        }
        cv.notify_all();
        {
            std::unique_lock<std::mutex> lk(mtx);
            while (state == PAUSED) cv.wait(lk); // wait the signal from control thread
            if (state == STEP) state = PAUSED;
            waiting = false;
        }
        cv.notify_all();
        time_point = std::chrono::high_resolution_clock::now();
        previous_timepoint_set = true;
    }
    void setState(int s) {
        {
            std::unique_lock<std::mutex> lk(mtx);
            state = s;
        }
        cv.notify_all();
    }
    void Show() {
        switch (state)
        {
        case RUNNING:
            if (ImGui::Button("pause")) setState(PAUSED);
            break;
        case PAUSED:
        case STEP:
            if (ImGui::Button("run")) setState(RUNNING);
            ImGui::SameLine();
            if (ImGui::Button("step")) setState(STEP);
            break;
        }
        const float * data = time_count.data();
        size_t size = time_count.size();
        if (size > 50) { data += size - 50; size = 50; }
        float sum = 0; for (int i = 0; i < size; i++) sum += data[i];
        ImGui::Text("average %.2f ms", sum / size);
        ImGui::PlotLines("plot", data, size);
    }
};

class ThreedbgApp : public Application {
public:
    ThreedbgApp(int width = 1280, int height = 720);
    ~ThreedbgApp();
    void loopOnce();
    void addDrawer(std::string name, std::unique_ptr<Drawer> d) {
        if (drawers.find(name) != drawers.end())
            drawers[name].ptr = std::move(d);
        else
            drawers[name] = {true, std::move(d)};
    }
    void snapshot(int & w, int & h, std::vector<unsigned char> & pixels) {
        draw();
        w = cam.resolution[0]; h = cam.resolution[1];
        pixels.resize(w* h * 3);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCheckError();
    }
    void barrier() {
        blk.barrier();
    }
private:
    DrawingCtx ctx;
    ImageViewer iv;
    Blocker blk;

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

ThreedbgApp::ThreedbgApp(int width, int height) : Application("3D debug", 0, width, height) {
    ImGui::StyleColorsLight();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9,0.9,0.9,0);
    PointsDrawer::initGL();
    LinesDrawer::initGL();
    glCheckError();
}
ThreedbgApp::~ThreedbgApp() {
    LinesDrawer::freeGL();
    PointsDrawer::freeGL();
    glCheckError();
}
void ThreedbgApp::loopOnce() {
    glCheckError();
    Application::newFrame();
    ImGui::ShowDemoWindow();
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

    if (ImGui::Begin("execute")) {
        blk.Show();
    }
    ImGui::End();

    //ImGui::ShowDemoWindow();

    Application::endFrame();
    glCheckError();
}

#include <thread>
#include <mutex>
#include <queue>

namespace threedbg {
// basicly ThreedbgApp + thread-safe drawerfactories as cache
bool showGui = true;
static std::thread displayThread;
class queued_lock {
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::thread::id> thread_queue;
    bool locked = false;
public:
    void lock() {
        std::unique_lock<std::mutex> lk(mtx);
        thread_queue.push(std::this_thread::get_id());
        while (locked || std::this_thread::get_id() != thread_queue.front())
            cv.wait(lk);
        thread_queue.pop();
        locked = true;
    }
    void unlock() {
        {
            std::unique_lock<std::mutex> lk(mtx);
            locked = false;
        }
        cv.notify_all();
    }
};

static queued_lock context_mtx;
static queued_lock cache_mtx;

static std::map<std::string, std::unique_ptr<DrawerFactory>> drawerFactories;
static std::unique_ptr<ThreedbgApp> app = nullptr;

static void flushDrawers() {
    std::map<std::string, std::unique_ptr<DrawerFactory>> dfs = std::move(drawerFactories);
    drawerFactories.clear();
    for (auto & p : dfs)
        app->addDrawer(p.first, std::unique_ptr<Drawer>(p.second->createDrawer()));
}

void init(void) {
    if (showGui) {
        displayThread = std::thread([&](void) { // new thread for opengl display
            context_mtx.lock();
            app = std::make_unique<ThreedbgApp>();
            app->show();
            app->unbindContext();
            context_mtx.unlock();
            while (!app->shouldClose()) {
                cache_mtx.lock();
                context_mtx.lock();
                app->bindContext();
                flushDrawers();
                app->loopOnce();
                app->unbindContext();
                context_mtx.unlock();
                cache_mtx.unlock();
                // limit fps
                static std::chrono::time_point<std::chrono::high_resolution_clock> prev_tp;
                std::this_thread::sleep_until(prev_tp + std::chrono::nanoseconds(1000000000 / 60));
                prev_tp = std::chrono::high_resolution_clock::now();
            }
            context_mtx.lock();
            app->bindContext();
            app.reset(nullptr);
            context_mtx.unlock();
        });
        while (!app) std::this_thread::yield();
    } else {
        app = std::make_unique<ThreedbgApp>();
        app->hide();
    }
}
void free(bool force) {
    context_mtx.lock();
    if (force && app) app->close();
    context_mtx.unlock();
    if (showGui) displayThread.join();
    else {
        app.reset(nullptr);
    }
}
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df) {
    cache_mtx.lock();
    drawerFactories[name] = std::move(df);
    cache_mtx.unlock();
}
bool working(void) {
    context_mtx.lock();
    if (showGui && app) app->barrier();
    bool r = app && !app->shouldClose();
    context_mtx.unlock();
    return r;
}
void snapshot(int & w, int & h, std::vector<unsigned char> & pixels) {
    cache_mtx.lock();
    context_mtx.lock();
    if (app) {
        app->bindContext();
        flushDrawers();
        app->snapshot(w, h, pixels);
        app->unbindContext();
    } else {
        w = h = 0; pixels.clear();
    }
    context_mtx.unlock();
    cache_mtx.unlock();
}
}
