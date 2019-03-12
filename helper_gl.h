#pragma once

#include <GL/gl.h>

#include <string>

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

struct DrawingCtx {
    GLuint fb; GLuint rb; GLuint texture;
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
