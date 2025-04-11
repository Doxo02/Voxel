cmake_minimum_required(VERSION 3.10.0)
project(Voxel VERSION 0.1.0 LANGUAGES C CXX)

set(OpenGL_GL_PREFERENCE "GLVND")
set(DOWNLOAD_EXTRACT_TIMESTAMP true)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

include(FetchContent)

FetchContent_Declare(
    imgui_external
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(imgui_external)

add_library(imgui
    ${imgui_external_SOURCE_DIR}/imgui.cpp
    ${imgui_external_SOURCE_DIR}/imgui_draw.cpp
	${imgui_external_SOURCE_DIR}/imgui_tables.cpp
	${imgui_external_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_external_SOURCE_DIR}/imgui_demo.cpp
	${imgui_external_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_external_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})

add_executable(Voxel main.cpp
    GL_Abstract/Program.cpp
    GL_Abstract/VertexArray.cpp
    GL_Abstract/VertexBuffer.cpp
    GL_Abstract/ElementBuffer.cpp
    GL_Abstract/ShaderStorageBuffer.cpp
    App.cpp
    Rendering/Camera.cpp)

target_include_directories(Voxel PUBLIC ${imgui_external_SOURCE_DIR})

target_link_libraries(Voxel PRIVATE GLEW::GLEW glfw glm OpenGL::GL imgui spdlog::spdlog)