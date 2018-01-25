#include "camera.h"

using namespace threedbg;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

static glm::fvec3 eye = {3, 2, 1}, center = {0, 0, 0}, up = {0, 0, 1};
static float fovy = 1;

void threedbg::camera::setEye(glm::fvec3 e) { eye = e; }
glm::fvec3 threedbg::camera::getEye(void) { return eye; }
void threedbg::camera::setCenter(glm::fvec3 c) { center = c; }
glm::fvec3 threedbg::camera::getCenter(void) { return center; }
void threedbg::camera::setUp(glm::fvec3 u) { up = glm::normalize(u); }
glm::fvec3 threedbg::camera::getUp(void) { return up; }
void threedbg::camera::setFovy(float f) { fovy = f; }
float threedbg::camera::getFovy(void) { return fovy; }

static float getDist(void) { return glm::length(eye - center); }

glm::fmat4 threedbg::camera::getProjMat(void) {
    float dist = getDist();
    return glm::perspective(fovy, display::getAspect(), .02f * dist,
                            50 * dist) *
           glm::lookAt(eye, center, up);
}

void threedbg::camera::rotateEye(float hangle, float vangle) {
    glm::fvec3 dir = glm::normalize(eye - center);
    dir = glm::rotate(dir, hangle, up);
    dir += vangle * up;
    dir = glm::normalize(dir);
    eye = center + getDist() * dir;
}
void threedbg::camera::rotateCenter(float hangle, float vangle) {
    glm::fvec3 dir = glm::normalize(center - eye);
    dir = glm::rotate(dir, hangle, up);
    dir += vangle * up;
    dir = glm::normalize(dir);
    center = eye + getDist() * dir;
}