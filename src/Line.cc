#include "Line.h"

#include "common.h"

using namespace threedbg::Line;
static std::mutex & lock = threedbg::globalLock;

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

void threedbg::Line::draw(void) {
    lock.lock();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(std::pair<glm::fvec3, glm::fvec3>) * lines.size(),
                 &lines[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * colors.size(),
                 &colors[0], GL_STREAM_DRAW);
    glLineWidth(1.5);
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
    return glm::fvec3(.7, .2 + r * .2, .1);
}

void threedbg::Line::add(Line l) {
    glm::fvec3 c = getColor();
    add(l, c);
}


void threedbg::Line::addAABB(glm::fvec3 min, glm::fvec3 max) {
    glm::fvec3 c = getColor();
    add(std::make_pair(glm::fvec3(min.x, min.y, min.z),
                       glm::fvec3(max.x, min.y, min.z)), c);
    add(std::make_pair(glm::fvec3(min.x, max.y, min.z),
                       glm::fvec3(max.x, max.y, min.z)), c);
    add(std::make_pair(glm::fvec3(min.x, min.y, max.z),
                       glm::fvec3(max.x, min.y, max.z)), c);
    add(std::make_pair(glm::fvec3(min.x, max.y, max.z),
                       glm::fvec3(max.x, max.y, max.z)), c);

    add(std::make_pair(glm::fvec3(min.x, min.y, min.z),
                       glm::fvec3(min.x, max.y, min.z)), c);
    add(std::make_pair(glm::fvec3(max.x, min.y, min.z),
                       glm::fvec3(max.x, max.y, min.z)), c);
    add(std::make_pair(glm::fvec3(min.x, min.y, max.z),
                       glm::fvec3(min.x, max.y, max.z)), c);
    add(std::make_pair(glm::fvec3(max.x, min.y, max.z),
                       glm::fvec3(max.x, max.y, max.z)), c);

    add(std::make_pair(glm::fvec3(min.x, min.y, min.z),
                       glm::fvec3(min.x, min.y, max.z)), c);
    add(std::make_pair(glm::fvec3(max.x, min.y, min.z),
                       glm::fvec3(max.x, min.y, max.z)), c);
    add(std::make_pair(glm::fvec3(min.x, max.y, min.z),
                       glm::fvec3(min.x, max.y, max.z)), c);
    add(std::make_pair(glm::fvec3(max.x, max.y, min.z),
                       glm::fvec3(max.x, max.y, max.z)), c);
}

void threedbg::Line::addAxes(glm::fvec3 pos, float size) {
    const glm::fvec3 r(1, 0, 0);
    const glm::fvec3 g(0, 1, 0);
    const glm::fvec3 b(0, 0, 1);
    add(std::make_pair(pos, pos + size * r), r);
    add(std::make_pair(pos, pos + size * g), g);
    add(std::make_pair(pos, pos + size * b), b);
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
size_t threedbg::Line::size(void) { return lineBuffer.size(); }
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