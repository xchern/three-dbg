#include <thread>
#include <chrono>

#include "src/threedbg.h"

std::vector<threedbg::Point::Point> cps;

int main(void) {
    threedbg::init();
    while (threedbg::working()) {
        printf("loop\n");
        threedbg::Point::clear();
        for (float x = -10; x <= 10; x++)
            for (float y = -10; y <= 10; y++)
                for (float z = -0; z <= 0; z++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    threedbg::Point::add(glm::fvec3(x / 10, y / 10, z / 10));
                    threedbg::Point::flush();
                }
    }
    printf("quit\n");
    threedbg::free();
}