#pragma once

#include "threedbg.h"


namespace threedbg {
namespace display {
void init(void);
bool finished(void);
void loopOnce(void);
void free(void);
float getAspect(void);
} // namespace display
} // namespace threedbg