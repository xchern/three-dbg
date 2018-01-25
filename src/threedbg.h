#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

namespace threedbg {
typedef glm::fvec3 Color;
void init(void);
void free(void);
bool working(void);
void wait(void);

namespace Point {
typedef glm::fvec3 Point;
void add(Point p);
void add(Point p, Color c);
void add(const std::vector<Point> &ps);
void add(const std::vector<Point> &ps, const std::vector<Color> &cs);
void clear(void);
void flush(void);
} // namespace Point
} // namespace threedbg