#pragma once

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace threedbg {
namespace display {
void init(void);
bool finished(void);
void loopOnce(void);
void free(void);
glm::fmat4 projMat(void);
} // namespace display
} // namespace threedbg