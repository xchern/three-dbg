#pragma once

#include "threedbg.h"

namespace threedbg {
namespace Line {
void init(void);
void draw(void);
void free(void);

typedef std::pair<glm::fvec3, glm::fvec3> Line;
void add(Line l);
void add(Line l, Color c);
void add(const std::vector<Line> &ls);
void add(const std::vector<Line> &ls, const std::vector<Color> &cs);
void addAABB(glm::fvec3 min, glm::fvec3 max);
void addAxes(glm::fvec3 pos, float size);
size_t size(void);
void clear(void);
void flush(void);
} // namespace Line
} // namespace threedbg