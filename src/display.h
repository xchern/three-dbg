#pragma once

#include "threedbg.h"


namespace threedbg {
namespace display {
void init(void);
bool finished(void);
void loopOnce(void);
void free(void);
float getAspect(void);
void setDisplaySize(int w, int h);
void getDisplaySize(int *w, int *h);
// w * h * 3 * sizeof(float) bytes will be filled
std::vector<glm::fvec3> getImage(void);
} // namespace display
} // namespace threedbg