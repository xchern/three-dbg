#include "camera.h"

using namespace threedbg;
using namespace threedbg::camera;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

static glm::fvec3 eye = {3, 1, 2}, center = {0, 0, 0}, up = {0, 0, 1};
static float fovy = 1;

void threedbg::camera::setEye(glm::fvec3 e) { eye = e; }
glm::fvec3 threedbg::camera::getEye(void) { return eye; }
void threedbg::camera::setCenter(glm::fvec3 c) { center = c; }
glm::fvec3 threedbg::camera::getCenter(void) { return center; }
void threedbg::camera::setUp(glm::fvec3 u) { up = glm::normalize(u); }
glm::fvec3 threedbg::camera::getUp(void) { return up; }
void threedbg::camera::setFovy(float f) { fovy = f; }
float threedbg::camera::getFovy(void) { return fovy; }

void camera::setDist(float d) { eye = center - getDir() * d; };
float camera::getDist(void) { return glm::length(eye - center); }
glm::fvec3 camera::getDir(void) { return glm::normalize(center - eye); }
glm::fvec3 camera::getTop(void) { return glm::cross(getRight(), getDir()); }
glm::fvec3 camera::getRight(void) { return glm::normalize(glm::cross(getDir(), up)); }

glm::fmat4 camera::getProjMat(void) {
    float dist = getDist();
    return glm::perspective(fovy, display::getAspect(), .02f * dist,
                            50 * dist) *
           glm::lookAt(eye, center, up);
}

void camera::rotateEye(float hangle, float vangle) {
    glm::fvec3 dir = -getDir();
    dir = glm::rotate(dir, hangle, up);
    dir += vangle * up;
    dir = glm::normalize(dir);
    eye = center + getDist() * dir;
}
void camera::rotateCenter(float hangle, float vangle) {
    glm::fvec3 dir = getDir();
    dir = glm::rotate(dir, hangle, up);
    dir += vangle * up;
    dir = glm::normalize(dir);
    center = eye + getDist() * dir;
}
void camera::translate(float r, float u) {
    glm::fvec3 dr = getTop() * u + getRight() * r;
    center += dr;
    eye += dr;
}