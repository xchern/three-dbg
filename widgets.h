#pragma once

#include "Application.h"

struct ImageViewer {
    enum { SCALE_ORIGIN, SCALE_FIT_FRAME, SCALE_FIT_WIDTH };
    int fit = SCALE_ORIGIN;
    void Show(GLuint texture, ImVec2 size) {
        if (ImGui::RadioButton("origin size", fit == SCALE_ORIGIN)) fit = SCALE_ORIGIN;
        ImGui::SameLine();
        if (ImGui::RadioButton("fit frame", fit == SCALE_FIT_FRAME)) fit = SCALE_FIT_FRAME;
        ImGui::SameLine();
        if (ImGui::RadioButton("fit width", fit == SCALE_FIT_WIDTH)) fit = SCALE_FIT_WIDTH;

        if (fit == SCALE_FIT_WIDTH) {
            float scale = ImGui::GetWindowContentRegionWidth() / size.x * 0.95f;
            size.x *= scale; size.y *= scale;
        } else if (fit == SCALE_FIT_FRAME) {
            float scale = fmin(ImGui::GetWindowContentRegionWidth() / size.x * 0.95f, ImGui::GetWindowHeight() / size.y * 0.9f);
            size.x *= scale; size.y *= scale;
        }

        ImGui::BeginChild("image", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        float x = fmax((ImGui::GetWindowContentRegionWidth() - size.x) * 0.5f, 0);
        ImGui::SameLine(x);
        ImGui::Image((ImTextureID)(long long)texture, size, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 1));
        ImGui::EndChild();
    }
};

#include <vector>
#include <mutex>
#include <condition_variable>

class ExecuteManager {
    int state;
    bool waiting = false;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<float> time_count;
    bool previous_timepoint_set = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
public:
    enum { RUNNING, PAUSED, STEP };
    ExecuteManager() : state(RUNNING) {}
    ~ExecuteManager() {
        setState(RUNNING);
        std::unique_lock<std::mutex> lk(mtx);
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

