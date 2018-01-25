cmake_minimum_required( VERSION 2.6 )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules/")

find_package( OpenGL REQUIRED )
find_package( PythonLibs REQUIRED)

include_directories(
    ${PYTHON_INCLUDE_DIRS}
    gl3w
    )

add_library(gl3w SHARED
    gl3w/GL/gl3w.c
    gl3w/GL/gl3w.h
    )
target_link_libraries(gl3w
    ${OPENGL_LIBRARIES}
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
    )
target_link_libraries(threedbg
    glfw
    gl3w
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