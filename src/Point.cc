#include "Point.h"

#include "common.h"

using namespace threedbg::Point;

static std::vector<Point> points;

static std::vector<threedbg::Color> colors;

static std::vector<Point> pointBuffer;
static std::vector<threedbg::Color> colorBuffer;

static std::mutex & lock = threedbg::globalLock;

static void addColor(void) {
    unsigned int a = colorBuffer.size();
    a = hash(a);
    a &= 0xff;
    float r = a * 1.0f / 255;
    colorBuffer.push_back(glm::fvec3(0, .4 + r * .2, 1));
}
void threedbg::Point::add(Point p) {
    pointBuffer.push_back(p);
    addColor();
}
void threedbg::Point::add(Point p, Color c) {
    pointBuffer.push_back(p);
    colorBuffer.push_back(c);
}
void threedbg::Point::add(const std::vector<Point> &ps) {
    pointBuffer.insert(pointBuffer.end(), ps.begin(), ps.end());
    while (colorBuffer.size() < pointBuffer.size())
        addColor();
}
void threedbg::Point::add(const std::vector<Point> &ps,
                          const std::vector<Color> &cs) {
    pointBuffer.insert(pointBuffer.end(), ps.begin(), ps.end());
    colorBuffer.insert(colorBuffer.end(), cs.begin(), cs.end());
}
void threedbg::Point::clear(void) {
    pointBuffer.clear();
    colorBuffer.clear();
}
void threedbg::Point::flush(void) {
    lock.lock();
    points = pointBuffer;
    colors = colorBuffer;
    lock.unlock();
}

static float pointSize = 5e-2;

void threedbg::Point::setPointSize(float p) { pointSize = p; }
float threedbg::Point::getPointSize(void) { return pointSize; }

const std::string vertex_shader_src = R"(
#version 330
uniform mat4 projMat;
uniform float pointSize;
uniform float pointPixels;

in vec3 vPos;
in vec3 vColor;
out vec3 fColor;
void main() {
    fColor = vColor;
    gl_Position = projMat * vec4(vPos.xyz,1);
    gl_PointSize = max(pointPixels / (gl_Position.w + 5 * pointSize), 1);
}
)";
const std::string fragment_shader_src = R"(
#version 330
in vec3 fColor;
out vec3 oColor;
void main() {
    vec2 pos = gl_PointCoord * 2 - 1;
    float l2  = dot(pos, pos);
    if (l2 > 1) discard;
    else oColor = fColor;
    if (l2 > 0.6) oColor *= 0.1;
}
)";

static GLuint program, vbo_pos, vbo_color, vao;

void threedbg::Point::init(void) {
    program = glCreateProgram();
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);
    glGenVertexArrays(1, &vao);

    compileShader(program, vertex_shader_src, fragment_shader_src);

    // bind
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

void threedbg::Point::free(void) {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_pos);
    glDeleteBuffers(1, &vbo_color);
    glDeleteVertexArrays(1, &vao);
    clear();
}

void threedbg::Point::draw(void) {
    float pointPixels = 0;
    {
        int w, h;
        display::getDisplaySize(&w, &h);
        pointPixels = h / camera::getFovy() * pointSize;
    }
    lock.lock();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * points.size(),
                 &points[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * colors.size(),
                 &colors[0], GL_STREAM_DRAW);
    glUseProgram(program);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glm::fmat4 projMat = threedbg::camera::getProjMat();
    glUniformMatrix4fv(glGetUniformLocation(program, "projMat"), 1, false, &projMat[0][0]);
    glUniform1f(glGetUniformLocation(program, "pointSize"), pointSize);
    glUniform1f(glGetUniformLocation(program, "pointPixels"), pointPixels);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, points.size());
    lock.unlock();
}
