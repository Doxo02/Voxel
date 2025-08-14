cmake_minimum_required(VERSION 3.10.0)
project(Voxel VERSION 0.1.0 LANGUAGES C CXX)

set(OpenGL_GL_PREFERENCE "GLVND")
set(DOWNLOAD_EXTRACT_TIMESTAMP true)
set(CXX_STANDARD 17)

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

add_library(VoxelEngine 
    src/Engine/vxe/Application.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_Shader.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_VertexArray.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_VertexBuffer.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_IndexBuffer.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_ShaderStorageBuffer.cpp
    src/Engine/vxe/Rendering/graphics/ogl/ogl_RenderAPI.cpp
    src/Engine/vxe/Rendering/graphics/Factories.cpp
    src/Engine/vxe/Rendering/Renderer.cpp
    src/Engine/vxe/Rendering/VoxelGrid.cpp
	src/Engine/vxe/DataStructures/Grid.cpp
	src/Engine/vxe/DataStructures/BrickMap.cpp
)

target_include_directories(VoxelEngine PUBLIC lib)
target_link_libraries(VoxelEngine PUBLIC
    GLEW::GLEW
    glm
    OpenGL::GL
    spdlog::spdlog
)

add_executable(VoxelApp
    #src/main.cpp
    src/App.cpp
    src/Rendering/Camera.cpp)

target_include_directories(VoxelApp PUBLIC ${imgui_external_SOURCE_DIR} lib src/Engine)

target_link_libraries(VoxelApp PRIVATE GLEW::GLEW glfw glm OpenGL::GL imgui spdlog::spdlog VoxelEngine)