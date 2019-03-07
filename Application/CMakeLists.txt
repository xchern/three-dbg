find_package(OpenGL REQUIRED)
find_package(GLFW3 CONFIG REQUIRED)

include_directories(
    .
    ${OPENGL_INCLUDE_DIR}
    )

add_library(Application
    CMakeLists.txt

    GL/gl3w.c

    Application.cc
    Application.h

    imconfig.h
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imconfig.h

    imgui_impl_glfw.cpp
    imgui_impl_glfw.h
    imgui_impl_opengl3.cpp
    imgui_impl_opengl3.h
    )

target_link_libraries(Application
    glfw
    ${OPENGL_LIBRARIES}
    )

if (UNIX)
    target_link_libraries(threedbg
        dl # used by gl3w for loading OpenGL functions
        pthread # multithread utilities in std:: namespace
        )
endif (UNIX)
