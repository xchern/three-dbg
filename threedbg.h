#pragma once

#include <memory>
#include "drawer.h"
#include "points.h"
#include "lines.h"

namespace threedbg {
void initDisplay(void);
void freeDisplay(void);
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df);
bool working(void);
Camera getCamera();
void setCamera(const Camera &);
}