find_package(OpenGL REQUIRED)

include_directories( ./Application )
add_subdirectory(Application)

add_library(threedbg
    lines.cc
    lines.h
    points.cc
    points.h
    drawer.h
    threedbg.cc
    threedbg.h
    )
target_link_libraries(threedbg
    Application
    )

add_executable(demo
    demo.cc
    )
target_link_libraries(demo
    threedbg
    )
