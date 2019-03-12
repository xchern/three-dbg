#pragma once

#include <GL/gl3w.h>
#include <string>
#include <map>

#include "camera.h"
#include "Application.h"

#include "helper_gl.h"

struct draw_param {
    float mat[4][4];
    Camera cam;
};

struct Drawer {
    virtual ~Drawer() {}
    virtual void draw(const struct draw_param &)=0;
};

struct DrawerFactory {
    virtual ~DrawerFactory() {}
    virtual Drawer * createDrawer()=0;
};
