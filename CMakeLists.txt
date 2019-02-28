find_package(OpenGL REQUIRED)
find_package(GLFW3 CONFIG REQUIRED)

include_directories(
    .
    ./imgui
    ${OPENGL_INCLUDE_DIR}
    )
add_subdirectory(imgui)

add_library(threedbg
    GL/gl3w.c
    ViewerApp.cc
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    lines.cc
    points.cc
    threedbg.cc
    )
target_link_libraries(threedbg
    ${OPENGL_LIBRARIES}
    glfw
    imgui
    )
if (UNIX)
    target_link_libraries(threedbg
        dl
        pthread
        )
endif (UNIX)

add_executable(demo
    demo.cc
    )
target_link_libraries(demo
    threedbg
    )
