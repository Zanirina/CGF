cmake_minimum_required(VERSION 3.10)
project(ComputerGraphicsAssignment)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/22208/vcpkg/scripts/buildsystems/vcpkg.cmake")

# --- Подключаем нужные пакеты ---
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# --- Assignment 2: main.cpp ---
add_executable(main main.cpp)
target_link_libraries(main PRIVATE OpenGL::GL glfw GLEW::GLEW)

# --- Assignment 3: cube.cpp (цветной 3D куб) ---
add_executable(cube cube.cpp)
target_link_libraries(cube PRIVATE GLEW::GLEW glfw OpenGL::GL)
