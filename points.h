#pragma once

#include "drawer.h"

#include <vector>
#include <glm/glm.hpp>

struct PointsDrawer : Drawer {
    static void initGL();
    static void freeGL();

    size_t particleNumber;
    float particleRadius;
    GLuint vao;
    GLuint buffers[2];

    PointsDrawer();
    virtual ~PointsDrawer() override;
    virtual void draw(const struct draw_param &) override;
};

struct PointsDrawerFactory : DrawerFactory {
    virtual Drawer * createDrawer() override {
        return createPointDrawer();
    }
    size_t particleNumber;
    float particleRadius;
    std::vector<glm::fvec3> pos, col;
    PointsDrawerFactory() : particleNumber(0), particleRadius(1) {}
    PointsDrawer * createPointDrawer();
    void addPoints(size_t n, glm::fvec3 * p, glm::fvec3 * c) {
        pos.insert(pos.end(), p, p+n);
        col.insert(col.end(), c, c+n);
        particleNumber += n;
    }
    void addPoint(glm::fvec3 p, glm::fvec3 c) {
        pos.push_back(p);
        col.push_back(c);
        particleNumber++;
    }
};
