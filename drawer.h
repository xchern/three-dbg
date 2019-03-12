#pragma once

#include <GL/gl3w.h>
#include <string>
#include <map>

#include "camera.h"
#include "Application.h"

#define errorln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

// opengl utilities

#define glCheckError()                                                          \
    do {                                                                        \
        GLenum err = glGetError();                                              \
        if (err != GL_NO_ERROR)                                                 \
            errorln("OpenGL error in file '%s' in line %i, Error Code 0x%x.",   \
                    __FILE__, __LINE__, err);                                   \
    } while (0)

inline bool compileShader(GLuint shader, int count, const char * srcs[]) {
    glShaderSource(shader, count, srcs, NULL);
    glCompileShader(shader);
    GLint isCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    return isCompiled;
}

inline bool compileShader(GLuint shader, const char * src) {
    return compileShader(shader, 1, &src);
}

inline bool linkProgram(GLuint program, GLuint vert_shdr, GLuint frag_shdr) {
    glAttachShader(program, vert_shdr);
    glAttachShader(program, frag_shdr);
    glLinkProgram(program);
    glDetachShader(program, vert_shdr);
    glDetachShader(program, frag_shdr);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    return isLinked;
}

inline std::string getShaderInfoLog(GLuint shader) {
    GLint length; // The length includes the NULL character
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string infoLog(length, '\0');
    glGetShaderInfoLog(shader, length, &length, &infoLog[0]);
    infoLog.resize(length);
    return infoLog;
}

inline std::string getProgramInfoLog(GLuint program) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::string infoLog(length, '\0');
    glGetProgramInfoLog(program, length, &length, &infoLog[0]);
    infoLog.resize(length);
    return infoLog;
}

inline int programFromSource(GLuint program, const char * vert_src, const char * frag_src) {
    int ret = 0;
    GLuint vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertex, vert_src)) {
        errorln("vertex shader log: %s", getShaderInfoLog(vertex).c_str());
        ret = 1; goto fail_vert;
    }
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragment, frag_src)) {
        errorln("fragment shader log: %s", getShaderInfoLog(fragment).c_str());
        ret = 1; goto fail_frag;
    }
    if (!linkProgram(program, vertex, fragment)) {
        errorln("program log: %s", getProgramInfoLog(program).c_str());
        ret = 1;
    }
fail_frag:
    glDeleteShader(fragment);
fail_vert:
    glDeleteShader(vertex);
    return ret;
}

// opengl drawing structures

struct draw_param {
    float mat[4][4];
    Camera cam;
};

struct Drawer {
    virtual ~Drawer() {}
    virtual void draw(const struct draw_param &)=0;
};

struct DrawerFactory {
    virtual ~DrawerFactory() {}
    virtual Drawer * createDrawer()=0;
};

struct DrawingCtx {
    GLuint fb;
    GLuint rb;
    GLuint texture;
    DrawingCtx() {
        glGenFramebuffers(1, &fb);
        glGenTextures(1, &texture);
        glGenRenderbuffers(1, &rb);
        glCheckError();

    }
    int resolution[2] = { 0,0 };
    void bindFB(int weight, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glCheckError();
        if (resolution[0] != weight || resolution[1] != height) {
            resolution[0] = weight; resolution[1] = height;
            // create a color attachment texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution[0], resolution[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            glCheckError();
            // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
            glBindRenderbuffer(GL_RENDERBUFFER, rb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution[0], resolution[1]); // use a single renderbuffer object for both a depth AND stencil buffer.
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb); // now actually attach it
            glCheckError();
            // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glViewport(0, 0, resolution[0], resolution[1]);
            glCheckError();
        }
    }
    ~DrawingCtx() {
        glDeleteFramebuffers(1, &fb);
        glDeleteTextures(1, &texture);
        glDeleteRenderbuffers(1, &rb);
        glCheckError();
    }
};

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

