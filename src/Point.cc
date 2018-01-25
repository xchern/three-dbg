#include "threedbg.h"
#include "display.h"
#include "Point.h"

#include <mutex>

using namespace threedbg::Point;

static std::vector<Point> points;
static std::vector<threedbg::Color> colors;
static std::mutex lock;

static std::vector<Point> pointBuffer;
static std::vector<threedbg::Color> colorBuffer;
void threedbg::Point::add(Point p) {
    pointBuffer.push_back(p);
    colorBuffer.push_back(glm::fvec3(1, 1, 1));
}
void threedbg::Point::add(const std::vector<Point> &ps) {
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
void threedbg::Point::flush(void){
    lock.lock();
    points = pointBuffer;
    colors = colorBuffer;
    lock.unlock();
}


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
    vec2 pos = gl_PointCoord - 0.5;
    float l2  = dot(pos, pos);
    if (l2 > 0.5 * 0.5) discard;
    if (l2 > 0.3 * 0.3) oColor = vec3(0);
    else oColor = fColor;
}
)";

static GLuint program, vbo_pos, vbo_color, vao;

void threedbg::Point::init(void) {
    program = glCreateProgram();
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_color);
    glGenVertexArrays(1, &vao);

    // compile shader
    GLint length;
    const GLchar * source;

    const GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    length = vertex_shader_src.size();
    source = vertex_shader_src.c_str();
    glShaderSource(vert, 1, &source, &length);

    const GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    length = fragment_shader_src.size();
    source = fragment_shader_src.c_str();
    glShaderSource(frag, 1, &source, &length);

    glCompileShader(vert);
    glCompileShader(frag);

    GLint isCompiled = 0;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled) goto error;
    glGetShaderiv(frag, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled) goto error;

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDetachShader(program, vert);
    glDetachShader(program, frag);
    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (!isLinked) goto error;

    glDeleteShader(vert);
    glDeleteShader(frag);

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

    return;
error:
    fprintf(stderr, "fail init OpenGL\n");
    exit(1);
}
void threedbg::Point::free(void) {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_pos);
    glDeleteBuffers(1, &vbo_color);
    glDeleteVertexArrays(1, &vao);
}

void threedbg::Point::draw(void) {
    lock.lock();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * points.size(),
                 &points[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::fvec3) * colors.size(),
                 &colors[0], GL_STREAM_DRAW);
    glPointSize(16);
    glUseProgram(program);
    glm::fmat4 projMat = threedbg::display::projMat();
    glUniformMatrix4fv(glGetUniformLocation(program, "projMat"), 1, false, &projMat[0][0]);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, points.size());
    lock.unlock();
}
