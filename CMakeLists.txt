cmake_minimum_required(VERSION 3.23)
project(BoidSimulationGpu
        VERSION 0.1
        DESCRIPTION "Template for cross-platform OpenGL cmake project"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include third-party libs
add_subdirectory(third_party/glew)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/stb_image)
add_subdirectory(third_party/glm)
add_subdirectory(third_party/imgui/imgui)

# Generate config.h
set(RESOURCE_PATH "${PROJECT_SOURCE_DIR}/res/")
configure_file(src/config.h.in src/config.h)

set(SOURCE_FILES
        "src/gl_util.h" "src/gl_util.cpp"
        src/simulation.cpp
        src/simulation.h
)

add_executable(OpenGL_cmake src/main.cpp ${SOURCE_FILES})

target_include_directories(OpenGL_cmake
        PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src"
        PRIVATE "src"
        PRIVATE "third_party/stb_image"
        PRIVATE "third_party/imgui")

target_link_libraries(OpenGL_cmake
        PRIVATE glfw
        PRIVATE libglew_static
        glm
        stb_image
        imgui)