#include "Line.h"

#include "common.h"

#include <mutex>

using namespace threedbg::Line;

const std::string vertex_shader_src = R"(
#version 330
uniform mat4 projMat;
in vec3 vPos;
in vec3 vColor;
out vec3 fColor;
void main() {
    fColor = vColor;
    gl_Position = projMat * vec4(vPos.xyz,1);
}
)";
const std::string fragment_shader_src = R"(
#version 330
in vec3 fColor;
out vec3 oColor;
void main() {
    oColor = fColor;
}
)";

static GLuint program, vbo_pos, vbo_color, vao;
void threedbg::Line::init(void) {
    program = glCreateProgram();
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);
    glGenVertexArrays(1, &vao);

    compileShader(program, vertex_shader_src, fragment_shader_src);

    GLint posLoc, colorLoc;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    posLoc = glGetAttribLocation(program, "vPos");
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(posLoc);
    colorLoc = glGetAttribLocation(program, "vColor");
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(colorLoc);
}

void threedbg::Line::free(void) {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_pos);
    glDeleteBuffers(1, &vbo_color);
    glDeleteVertexArrays(1, &vao);
    clear();
}

static std::vector<Line> lines;
static std::vector<threedbg::Color> colors;
static std::mutex lock;

void threedbg::Line::draw(void) {
    lock.lock();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(std::pair<glm::fvec3, glm::fvec3>) * lines.size(),
                 &lines[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * colors.size(),
                 &colors[0], GL_STREAM_DRAW);
    glUseProgram(program);
    glm::fmat4 projMat = threedbg::camera::getProjMat();
    glUniformMatrix4fv(glGetUniformLocation(program, "projMat"), 1, false, &projMat[0][0]);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2 * lines.size());
    lock.unlock();
}

static std::vector<Line> lineBuffer;
static std::vector<threedbg::Color> colorBuffer;

static glm::fvec3 getColor(void) {
    unsigned int a = lineBuffer.size();
    a = hash(a);
    a &= 0xff;
    float r = a * 1.0f / 255;
    return glm::fvec3(1, .3 + r * .4, 0);
}

void threedbg::Line::add(Line l) {
    glm::fvec3 c = getColor();
    add(l, c);
}

void threedbg::Line::add(Line l, Color c) {
    lineBuffer.push_back(l);
    colorBuffer.push_back(c);
    colorBuffer.push_back(c);
}
void threedbg::Line::add(const std::vector<Line> &ls) {
    glm::fvec3 c = getColor();
    for (auto l : ls)
        add(l, c);
}
void threedbg::Line::add(const std::vector<Line> &ls, const std::vector<Color> &cs) {
    lineBuffer.insert(lineBuffer.end(), ls.begin(), ls.end());
    colorBuffer.insert(colorBuffer.end(), cs.begin(), cs.end());
}
void threedbg::Line::clear(void) {
    lineBuffer.clear();
    colorBuffer.clear();
}
void threedbg::Line::flush(void) {
    lock.lock();
    lines = lineBuffer;
    colors = colorBuffer;
    lock.unlock();
}