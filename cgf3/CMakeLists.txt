cmake_minimum_required(VERSION 3.20)
project(cgf3)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(cgf3 main.cpp)

target_link_libraries(cgf3 PRIVATE glfw glm::glm GLEW::GLEW OpenGL::GL)
