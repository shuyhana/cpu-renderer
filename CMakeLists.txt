cmake_minimum_required(VERSION 3.14)
project(cpu_renderer LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cpu_renderer_core STATIC src/render/framebuffer.cpp)
target_include_directories(cpu_renderer_core PUBLIC include)
target_compile_features(cpu_renderer_core PUBLIC cxx_std_17)

find_package(X11 REQUIRED)

add_executable(cpu-renderer
    src/app/main.cpp
    src/platform/x11/window.cpp
)
target_include_directories(cpu-renderer PRIVATE src)
target_link_libraries(cpu-renderer PRIVATE cpu_renderer_core X11::X11)
