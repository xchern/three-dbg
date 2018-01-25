#pragma once

#include "threedbg.h"

namespace threedbg {
namespace camera {
glm::fmat4 getProjMat(void);

void setEye(glm::fvec3 eye);
glm::fvec3 getEye(void);
void setCenter(glm::fvec3 center);
glm::fvec3 getCenter(void);
void setUp(glm::fvec3 up);
glm::fvec3 getUp(void);
void setFovy(float fovy);
float getFovy(void);

float getDist(void);
glm::fvec3 getDir(void);
glm::fvec3 getTop(void);
glm::fvec3 getRight(void);

void rotateEye(float hangle, float vangle);
void rotateCenter(float hangle, float vangle);
void translate(float right, float up);
} // namespace camera
} // namespace threedbg