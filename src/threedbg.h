#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <mutex>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

namespace threedbg {
extern std::mutex globalLock;
typedef glm::fvec3 Color;
void init(void);
void free(void);
bool working(void);
void wait(void);
} // namespace threedbg

#include "camera.h"
#include "display.h"
#include "Point.h"
#include "Line.h"