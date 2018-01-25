//#include <Eigen/Core.h>

#include <pybind11/pybind11.h>

#include "src/threedbg.h"

namespace py = pybind11;

using namespace threedbg;

PYBIND11_MODULE(threedbg, m) {
    m.doc() = R"(Easy debuging 3d graphics with numpy and OpenGL.)";
    m.def("init", &threedbg::init, "init threedbg opengl display");
    m.def("free", &threedbg::free, "free resource");
    m.def("wait", &threedbg::wait, "wait for window closing and free resource");
}