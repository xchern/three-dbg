cmake_minimum_required( VERSION 2.6 )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules/")

find_package( OpenGL REQUIRED )
find_package( PythonLibs REQUIRED)

include_directories(
    ${PYTHON_INCLUDE_DIRS}
    gl3w
    external/pybind11/include/
    )

add_library(threedbg SHARED
    src/threedbg.cc
    src/threedbg.h
    src/display.cc
    src/display.h
    src/camera.cc
    src/camera.h
    src/Point.cc
    src/Point.h
    src/Line.cc
    src/Line.h
    src/common.h
    gl3w/GL/gl3w.c
    gl3w/GL/gl3w.h
    )
target_link_libraries(threedbg
    ${OPENGL_LIBRARIES}
    glfw
    )

add_library(py-threedbg SHARED
    python-binding.cc
    )
target_link_libraries(py-threedbg
    ${PYTHON_LIBRARIES}
    threedbg
    )
set_target_properties( py-threedbg PROPERTIES
    PREFIX ""
    OUTPUT_NAME "threedbg"
    )

add_executable(demo
    demo.cc
    )
target_link_libraries(demo
    threedbg
    )
