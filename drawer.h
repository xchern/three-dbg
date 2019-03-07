#pragma once

#include <GL/gl3w.h>
#include <string>

#include "camera.h"

#define errorln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

#define glCheckError()                                                          \
    do {                                                                        \
        GLenum err = glGetError();                                              \
        if (err != GL_NO_ERROR)                                                 \
            errorln("OpenGL error in file '%s' in line %i, Error Code 0x%x.", \
                    __FILE__, __LINE__, err);                                   \
    } while (0)

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
