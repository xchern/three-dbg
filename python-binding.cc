//#include <Eigen/Core.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "src/threedbg.h"

namespace py = pybind11;

using namespace threedbg;

PYBIND11_MODULE(threedbg, m) {
    m.doc() = R"(Easy debuging 3d graphics with numpy and OpenGL.)";
    m.def("init", &threedbg::init, "init threedbg opengl display");
    m.def("free", &threedbg::free, "free resource");
    m.def("wait", &threedbg::wait, "wait for window closing and free resource");
    m.def("working", &threedbg::working, "wheather the window is still open");

    py::module Point = m.def_submodule("Point", "Draw Points");
    Point.def("add", [](py::buffer buf){
        py::buffer_info info = buf.request();
        if (info.format != py::format_descriptor<float>::format())
            throw std::runtime_error("float array should be used to add points");
        if (info.ndim != 2 || info.shape[1] != 3)
            throw std::runtime_error("array should be of shape (x, 3) to add points");
        for (size_t i = 0; i < info.shape[0]; i++) {
            const float x = *(float *)((char *)info.ptr + i * info.strides[0] + 0 * info.strides[1]);
            const float y = *(float *)((char *)info.ptr + i * info.strides[0] + 1 * info.strides[1]);
            const float z = *(float *)((char *)info.ptr + i * info.strides[0] + 2 * info.strides[1]);
            Point::add(glm::fvec3(x, y, z));
        }
    }, "Add points");
    Point.def("add", [](py::buffer pos, py::buffer color){
        py::buffer_info posInfo = pos.request();
        py::buffer_info colorInfo = color.request();
        if (posInfo.format != py::format_descriptor<float>::format() ||
            colorInfo.format != py::format_descriptor<float>::format())
            throw std::runtime_error("float array should be used to add points");
        if (posInfo.ndim != 2 || posInfo.shape[1] != 3)
            throw std::runtime_error("array should be of shape (x, 3) to add points");
        if (posInfo.ndim != colorInfo.ndim || posInfo.shape != colorInfo.shape)
            throw std::runtime_error("shape of position and color should match");
        for (size_t i = 0; i < posInfo.shape[0]; i++) {
            const float x = *(float *)((char *)posInfo.ptr + i * posInfo.strides[0] + 0 * posInfo.strides[1]);
            const float y = *(float *)((char *)posInfo.ptr + i * posInfo.strides[0] + 1 * posInfo.strides[1]);
            const float z = *(float *)((char *)posInfo.ptr + i * posInfo.strides[0] + 2 * posInfo.strides[1]);
            const float r = *(float *)((char *)colorInfo.ptr + i * colorInfo.strides[0] + 0 * colorInfo.strides[1]);
            const float g = *(float *)((char *)colorInfo.ptr + i * colorInfo.strides[0] + 1 * colorInfo.strides[1]);
            const float b = *(float *)((char *)colorInfo.ptr + i * colorInfo.strides[0] + 2 * colorInfo.strides[1]);
            Point::add(glm::fvec3(x, y, z), glm::fvec3(r, g, b));
        }
    }, "Add points");
    Point.def("clear", &Point::clear, "Clear points");
    Point.def("flush", &Point::flush, "Flush points");
}