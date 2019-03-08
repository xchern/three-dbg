#pragma once

#include <memory>
#include <vector>
#include "drawer.h"
#include "points.h"
#include "lines.h"

namespace threedbg {
extern bool multithread;
void initDisplay(void);
void freeDisplay(bool force = false);
void addDrawerFactory(std::string name, std::unique_ptr<DrawerFactory> && df);
bool working(void);
void snapshot(int & w, int & h, std::vector<unsigned char> & pixels);
}
