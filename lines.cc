#include "lines.h"

static const char vert_src[] = R"(
#version 330
uniform mat4 VP;
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;
out vec3 fColor;
void main() {
    fColor = vColor;
    gl_Position = VP * vec4(vPos.xyz,1);
}
)";

static const char frag_src[] = R"(
#version 330
in vec3 fColor;
out vec3 oColor;
void main() {
    oColor = fColor;
}
)";

static GLuint program;

void LinesDrawer::initGL() {
    program = glCreateProgram();
    if (programFromSource(program, vert_src, frag_src))
        abort();
    glCheckError();
}
void LinesDrawer::freeGL() {
    glDeleteProgram(program);
    glCheckError();
}

LinesDrawer::LinesDrawer() {
    glGenBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    const static int bufferDim[] = {3, 3};
    for (int i = 0; i < (sizeof(buffers)/sizeof(buffers[0])); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
        glVertexAttribPointer(i, bufferDim[i], GL_FLOAT, GL_FALSE, bufferDim[i] * sizeof(float), (void *)0);
        glEnableVertexAttribArray(i);
    }
    glBindVertexArray(0);
    glCheckError();
}

void LinesDrawer::draw(const struct draw_param & dp) {
    glUseProgram(program);
    int VPLoc = glGetUniformLocation(program, "VP");
    glUniformMatrix4fv(VPLoc, 1, GL_FALSE, &dp.mat[0][0]);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vertexNumber);
    glCheckError();
}

LinesDrawer::~LinesDrawer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
    glCheckError();
}

LinesDrawer * LinesDrawerFactory::createLineDrawer() {
    LinesDrawer * p = new LinesDrawer();
    p->vertexNumber = vertexNumber;
    glBindBuffer(GL_ARRAY_BUFFER, p->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(pos[0]), pos.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, p->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(col[0]), col.data(), GL_DYNAMIC_DRAW);
    glCheckError();
    return p;
}
