#include "points.h"

static const char vert_src[] = R"(
#version 330
uniform mat4 VP;
uniform float unitSize;
uniform float radius;
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vCol;
out float fDepthA;
out float fDepthB;
out float fDist;
out float fBallRadius;
out float fEdgeWidth;
out vec3 fCol;
void main() {
    gl_Position = VP * vec4(vPos, 1.0);
    float sizeFactor = 1.0 / (1.0 + 3.0 * radius/gl_Position.w);
    gl_PointSize = unitSize * radius * sizeFactor / gl_Position.w;
    fEdgeWidth = min(4.0/gl_PointSize, 0.3);
    fDist = gl_Position.w;
    fDepthA = -length(vec3(VP[0][2], VP[1][2], VP[2][2])) / length(vec3(VP[0][3], VP[1][3], VP[2][3]));
    fDepthB = VP[3][2] + VP[3][3] * fDepthA;
    fBallRadius = radius * sizeFactor;
    fCol = vCol;
}
)";

static const char frag_src[] = R"(
#version 330
in float fDepthA;
in float fDepthB;
in float fDist;
in float fBallRadius;
in float fEdgeWidth;
in vec3 fCol;
void main() {
    vec2 pc = gl_PointCoord * 2.0 - 1.0;
    float l2 = dot(pc, pc);
    if (l2 > 1) discard;
    float dist = fDist - fBallRadius * sqrt(1.0 - l2);
    gl_FragDepth = -fDepthA + fDepthB / dist;
    gl_FragDepth = (gl_FragDepth + 1.0) / 2.0; // [-1, 1] normalized to [0, 1]
    vec3 col = fCol;
    col *= 1.0 - smoothstep(1.0-fEdgeWidth*2.0,1.0-fEdgeWidth,l2) + smoothstep(1.0-fEdgeWidth,1.0,l2);
    gl_FragColor = vec4(col, 1.0);
}
)";

static GLuint program;

void PointsDrawer::initGL() {
    program = glCreateProgram();
    if (programFromSource(program, vert_src, frag_src))
        abort();
    glCheckError();
}
void PointsDrawer::freeGL() {
    glDeleteProgram(program);
    glCheckError();
}

PointsDrawer::PointsDrawer() {
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

void PointsDrawer::draw(const struct draw_param & dp) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glUseProgram(program);
    int VPLoc = glGetUniformLocation(program, "VP");
    glUniformMatrix4fv(VPLoc, 1, GL_FALSE, &dp.mat[0][0]);
    int vUnitSizeLoc = glGetUniformLocation(program, "unitSize");
    glUniform1f(vUnitSizeLoc, dp.resolution[1]/dp.cam.getFovy());
    int radiusLoc = glGetUniformLocation(program, "radius");
    glUniform1f(radiusLoc, particleRadius);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, particleNumber);
    glCheckError();
}

PointsDrawer::~PointsDrawer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
    glCheckError();
}

PointsDrawer * PointsDrawerFactory::createPointDrawer() {
    PointsDrawer * p = new PointsDrawer();
    p->particleNumber = particleNumber;
    p->particleRadius = particleRadius;
    glBindBuffer(GL_ARRAY_BUFFER, p->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(pos[0]), pos.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, p->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(col[0]), col.data(), GL_DYNAMIC_DRAW);
    glCheckError();
    return p;
}
