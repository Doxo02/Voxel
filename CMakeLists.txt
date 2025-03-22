cmake_minimum_required(VERSION 3.10.0)
project(Voxel VERSION 0.1.0 LANGUAGES C CXX)

set(OpenGL_GL_PREFERENCE "GLVND")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(Voxel main.cpp
    GL_Abstract/Program.cpp
    GL_Abstract/VertexArray.cpp
    GL_Abstract/VertexBuffer.cpp
    GL_Abstract/ElementBuffer.cpp
    Rendering/Camera.cpp)

target_link_libraries(Voxel PRIVATE GLEW::GLEW glfw glm OpenGL::GL)