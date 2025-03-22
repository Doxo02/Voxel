cmake_minimum_required(VERSION 3.10.0)
project(Voxel VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(Voxel main.cpp)

target_link_libraries(Voxel PRIVATE OpenGL::GL GLEW::GLEW glfw)