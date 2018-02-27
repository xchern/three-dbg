//#include <Eigen/Core.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "src/threedbg.h"

namespace py = pybind11;

using namespace threedbg;

glm::fvec3 convert(py::object &&o) {
    auto t = o.cast<std::tuple<float, float, float>>();
    return glm::fvec3(std::get<0>(t), std::get<1>(t), std::get<2>(t));
}

inline py::object convert(glm::fvec3 &&v) {
    auto t = std::make_tuple(v.x, v.y, v.z);
    return py::cast(t);
}

PYBIND11_MODULE(threedbg, m) {
    m.doc() = R"(Easy debuging 3d graphics with numpy and OpenGL.)";
    m.def("init", &threedbg::init, "init threedbg opengl display");
    m.def("free", &threedbg::free, "free resource");
    m.def("wait", &threedbg::wait, "wait for window closing and free resource");
    m.def("working", &threedbg::working, "wheather the window is still open");

    py::module Point = m.def_submodule("Point", "Draw Points");
    Point.def("getPointSize", &Point::getPointSize, "get point size");
    Point.def("setPointSize", &Point::setPointSize, "set point size");
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

    py::module Line = m.def_submodule("Line", "Draw Lines");
    Line.def("add", [](py::array_t<float> source, py::array_t<float> target) {
        py::buffer_info sourceInfo = source.request();
        py::buffer_info targetInfo = target.request();
        if (sourceInfo.ndim != 2 || sourceInfo.shape[1] != 3 ||
            targetInfo.ndim != 2 || targetInfo.shape[1] != 3)
            throw std::runtime_error("source and target array should be of shape (x, 3)");
        if (sourceInfo.shape[0] != targetInfo.shape[0])
            throw std::runtime_error("source and target array should match in length");
        for (size_t i = 0; i < sourceInfo.shape[0]; i++) {
            const float sx = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 0 * sourceInfo.strides[1]);
            const float sy = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 1 * sourceInfo.strides[1]);
            const float sz = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 2 * sourceInfo.strides[1]);
            const float tx = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 0 * targetInfo.strides[1]);
            const float ty = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 1 * targetInfo.strides[1]);
            const float tz = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 2 * targetInfo.strides[1]);
            std::pair<glm::fvec3, glm::fvec3> l =
                std::make_pair(glm::fvec3(sx, sy, sz), glm::fvec3(tx, ty, tz));
            Line::add(l);
        }
    }, "Add Lines");
    Line.def("addAABB", [](py::array_t<float> mins, py::array_t<float> maxs) {
        py::buffer_info sourceInfo = mins.request();
        py::buffer_info targetInfo = maxs.request();
        if (sourceInfo.ndim != 2 || sourceInfo.shape[1] != 3 ||
            targetInfo.ndim != 2 || targetInfo.shape[1] != 3)
            throw std::runtime_error("source and target array should be of shape (x, 3)");
        if (sourceInfo.shape[0] != targetInfo.shape[0])
            throw std::runtime_error("source and target array should match in length");
        for (size_t i = 0; i < sourceInfo.shape[0]; i++) {
            const float sx = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 0 * sourceInfo.strides[1]);
            const float sy = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 1 * sourceInfo.strides[1]);
            const float sz = *(float *)((char *)sourceInfo.ptr + i * sourceInfo.strides[0] + 2 * sourceInfo.strides[1]);
            const float tx = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 0 * targetInfo.strides[1]);
            const float ty = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 1 * targetInfo.strides[1]);
            const float tz = *(float *)((char *)targetInfo.ptr + i * targetInfo.strides[0] + 2 * targetInfo.strides[1]);
            auto min = glm::fvec3(sx, sy, sz);
            auto max = glm::fvec3(tx, ty, tz);
            Line::addAABB(min, max);
        }
    }, "Add AABB Boxes");
    Line.def("addAxes",
             [](float x, float y, float z, float size) {
                 Line::addAxes(glm::fvec3(x, y, z), size);
             },
             "Show axes x,y,z at position");
    Line.def("clear", [](void){Line::clear();}, "Clear Lines");
    Line.def("flush", [](void){Line::flush();}, "Flush Lines");

    py::module camera = m.def_submodule("camera", "Interact with camera");
    camera.def("dump", [](void) {
        py::dict d;
        d["eye"] = convert(camera::getEye());
        d["center"] = convert(camera::getCenter());
        d["up"] = convert(camera::getUp());
        d["fovy"] = camera::getFovy();
        return d;
    }, "dump camera state");
    camera.def("load", [](py::dict d) {
        camera::setEye(convert(d["eye"]));
        camera::setCenter(convert(d["center"]));
        camera::setUp(convert(d["up"]));
        camera::setFovy(d["fovy"].cast<float>());
        return d;
    }, "dump camera state");

    py::module display = m.def_submodule("display", "manipulate the display");
    display.def("setDisplaySize",
                [](int width, int height) {
                    display::setDisplaySize(width, height);
                },
                "resize the window");
    display.def("getImage",
                [](void) {
                    auto pixels = display::getImage();
                    int w, h;
                    display::getDisplaySize(&w, &h);
                    py::array_t<float> img({h, w, 3});
                    memcpy(img.request().ptr, pixels.data(),
                           w * h * 3 * sizeof(float));
                    return img;
                },
                "snapshot the screen");
    display.def("getChar", &display::getChar, "get keyboard event");
}
