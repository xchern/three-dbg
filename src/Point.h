#pragma once

#include "threedbg.h"

namespace threedbg {
namespace Point {
void init(void);
void draw(void);
void free(void);

typedef glm::fvec3 Point;
void add(Point p);
void add(Point p, Color c);
void add(const std::vector<Point> &ps);
void add(const std::vector<Point> &ps, const std::vector<Color> &cs);
void clear(void);
void flush(void);
void setPointSize(float p);
float getPointSize(void);
} // namespace Point
} // namespace threedbg
