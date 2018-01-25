#include <thread>
#include <chrono>

#include "src/threedbg.h"

std::vector<threedbg::Point::Point> cps;

int main(void) {
    threedbg::init();
    while (threedbg::working()) {
        printf("loop\n");
        threedbg::Point::clear();
        for (float z = -5; z <= 5; z++)
            for (float y = -5; y <= 5; y++)
                for (float x = -5; x <= 5; x++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    threedbg::Point::add(glm::fvec3(x / 5, y / 5, z / 5));
                    threedbg::Point::flush();
                    threedbg::camera::rotateEye(.002, 0);
                }
    }
    printf("quit\n");
    threedbg::free();
}