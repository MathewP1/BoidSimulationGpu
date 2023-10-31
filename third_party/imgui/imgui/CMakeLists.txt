set(IMGUI_NAME imgui)

set(IMGUI_SOURCES
        "imgui.cpp"
        "imgui_demo.cpp"
        "imgui_draw.cpp"
        "imgui_widgets.cpp"
        "imgui_impl_glfw.cpp"
        "imgui_tables.cpp"
        "imgui_impl_opengl3.cpp")

set(IMGUI_HEADERS
        "imconfig.h"
        "imgui.h"
        "imgui_internal.h"
        "imstb_rectpack.h"
        "imstb_textedit.h"
        "imstb_truetype.h"
        "imgui_impl_glfw.h"
        "imgui_impl_opengl3.h"
        "imgui_impl_opengl3_loader.h")

add_library(${IMGUI_NAME}
        STATIC
        ${IMGUI_SOURCES}
        ${IMGUI_HEADERS})

target_link_libraries(${IMGUI_NAME}
        PRIVATE glfw)

add_executable(imgui-example
        main.cpp)

target_link_libraries(imgui-example
        PRIVATE ${IMGUI_NAME}
        PRIVATE glfw
        PRIVATE libglew_static)
