#pragma once

#include "drawer.h"

#include <vector>
#include <glm/glm.hpp>

struct LinesDrawer : Drawer {
    static void initGL();
    static void freeGL();

    size_t vertexNumber;
    GLuint vao;
    GLuint buffers[2];

    LinesDrawer();
    virtual ~LinesDrawer() override;
    virtual void draw(const struct draw_param &) override;
};

struct LinesDrawerFactory : DrawerFactory {
    virtual Drawer * createDrawer() override {
        return createLineDrawer();
    }
    size_t vertexNumber;
    std::vector<glm::fvec3> pos, col;
    LinesDrawerFactory() : vertexNumber(0) {}
    LinesDrawer * createLineDrawer();
    void addLine(glm::fvec3 p1, glm::fvec3 p2, glm::fvec3 c) {
        pos.push_back(p1); pos.push_back(p2);
        col.push_back(c); col.push_back(c);
        vertexNumber+=2;
    }
    void addAxes(glm::fvec3 ori, float l) {
        addLine(ori, ori + glm::fvec3(l,0,0), glm::fvec3(1,0,0));
        addLine(ori, ori + glm::fvec3(0,l,0), glm::fvec3(0,1,0));
        addLine(ori, ori + glm::fvec3(0,0,l), glm::fvec3(0,0,1));
    }
    void addAABB(glm::fvec3 min, glm::fvec3 max, glm::fvec3 c) {
        addLine(glm::fvec3(min.x,min.y,min.z), glm::fvec3(max.x,min.y,min.z), c);
        addLine(glm::fvec3(min.x,min.y,max.z), glm::fvec3(max.x,min.y,max.z), c);
        addLine(glm::fvec3(min.x,max.y,max.z), glm::fvec3(max.x,max.y,max.z), c);
        addLine(glm::fvec3(min.x,max.y,min.z), glm::fvec3(max.x,max.y,min.z), c);

        addLine(glm::fvec3(min.x,min.y,min.z), glm::fvec3(min.x,max.y,min.z), c);
        addLine(glm::fvec3(min.x,min.y,max.z), glm::fvec3(min.x,max.y,max.z), c);
        addLine(glm::fvec3(max.x,min.y,max.z), glm::fvec3(max.x,max.y,max.z), c);
        addLine(glm::fvec3(max.x,min.y,min.z), glm::fvec3(max.x,max.y,min.z), c);

        addLine(glm::fvec3(min.x,min.y,min.z), glm::fvec3(min.x,min.y,max.z), c);
        addLine(glm::fvec3(min.x,max.y,min.z), glm::fvec3(min.x,max.y,max.z), c);
        addLine(glm::fvec3(max.x,max.y,min.z), glm::fvec3(max.x,max.y,max.z), c);
        addLine(glm::fvec3(max.x,min.y,min.z), glm::fvec3(max.x,min.y,max.z), c);
    }
};
