#include <thread>
#include <chrono>
#include <iostream>

#include "src/threedbg.h"

std::vector<threedbg::Point::Point> cps;

int main(void) {
    threedbg::init();
    //threedbg::display::setDisplaySize(800, 600);
    threedbg::Line::clear();
    threedbg::Line::addAABB(glm::fvec3(-1, -1, -1), glm::fvec3(1, 1, 1));
    threedbg::Line::addAxes(glm::fvec3(0, 0, 0), 1);
    threedbg::Line::flush();
    threedbg::Line::clear();
    printf("looping\n");
    for (float t = 0; threedbg::working(); t += 1e-2) {
        const float z = sinf(2 * M_PI / 5.f * t);
        threedbg::Point::clear();
        for (float x = -5; x <= 5; x++)
            for (float y = -5; y <= 5; y++)
                threedbg::Point::add(glm::fvec3(x / 5, y / 5, z));
        threedbg::Point::flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        threedbg::camera::rotateEye(.001, 0);
        auto pixels = threedbg::display::getImage();
        int w, h; threedbg::display::getDisplaySize(&w, &h);
        std::cout << pixels.size() << ' ' << w << ' ' << h << std::endl;
    }
    printf("quit\n");
    threedbg::free();
}