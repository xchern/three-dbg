#include <thread>
#include <chrono>

#include "src/threedbg.h"

std::vector<threedbg::Point::Point> cps;

int main(void) {
    threedbg::init();
    while (threedbg::working()) {
        printf("loop\n");
        threedbg::Line::clear();
        threedbg::Line::add(std::make_pair(glm::fvec3(0,0,-1), glm::fvec3(0,0,1)));
        threedbg::Line::add(std::make_pair(glm::fvec3(0,-1,0), glm::fvec3(0,1,0)));
        threedbg::Line::add(std::make_pair(glm::fvec3(-1,0,0), glm::fvec3(1,0,0)));
        threedbg::Line::flush();
        for (float z = -5; z <= 5; z++)
            for (float y = -5; y <= 5; y++)
                for (float x = -5; x <= 5; x++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    threedbg::Point::add(glm::fvec3(x / 5, y / 5, z / 5));
                    threedbg::Point::flush();
                    threedbg::camera::rotateEye(.001, 0);
                }
    }
    printf("quit\n");
    threedbg::free();
}