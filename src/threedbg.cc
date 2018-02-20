#include "threedbg.h"
#include "display.h"

#include <map>
#include <thread>

using namespace threedbg;

static std::thread displayThread;
static bool isDisplayThreadOn = false;
static bool done;

std::mutex threedbg::globalLock;

void threedbg::init(void) {
    // new thread for opengl display
    if (isDisplayThreadOn) return;
    done = false;
    displayThread = std::thread([](void) {
        display::init();
        while (!done && !display::finished()) {
            display::loopOnce();
        }
        display::free();
    });
    isDisplayThreadOn = true;
    // wait util the display starts
    while (display::finished())
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void threedbg::free(void) {
    done = true;
    wait();
}

bool threedbg::working(void) { return !display::finished(); }
void threedbg::wait(void) {
    if (isDisplayThreadOn) displayThread.join();
    isDisplayThreadOn = false;
}