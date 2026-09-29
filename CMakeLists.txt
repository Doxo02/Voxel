cmake_minimum_required(VERSION 3.21)
project(Voxel VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(OpenGL_GL_PREFERENCE "GLVND")
set(DOWNLOAD_EXTRACT_TIMESTAMP true)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)

add_library(VoxelEngine 
    src/vxe/Application.cpp
    src/vxe/Platform/OpenGL/ogl_Shader.cpp
    src/vxe/Platform/OpenGL/ogl_VertexArray.cpp
    src/vxe/Platform/OpenGL/ogl_VertexBuffer.cpp
    src/vxe/Platform/OpenGL/ogl_IndexBuffer.cpp
    src/vxe/Platform/OpenGL/ogl_ShaderStorageBuffer.cpp
    src/vxe/Platform/OpenGL/ogl_RenderAPI.cpp
    src/vxe/Rendering/graphics/Factories.cpp
    src/vxe/Rendering/Renderer.cpp
    src/vxe/Rendering/VoxelGrid.cpp
	src/vxe/DataStructures/Grid.cpp
	src/vxe/DataStructures/BrickMap.cpp
    src/vxe/Core/Window.cpp
    src/vxe/Platform/Linux/LinuxWindow.cpp
    src/vxe/Platform/Windows/WindowsWindow.cpp
)

target_include_directories(VoxelEngine PUBLIC lib)
target_link_libraries(VoxelEngine PUBLIC
    GLEW::GLEW
    glm::glm
    OpenGL::GL
    spdlog::spdlog
)

add_executable(VoxelApp
    example/App.cpp
    example/Rendering/Camera.cpp
    example/Rendering/DebugInfo.cpp)

target_include_directories(VoxelApp PUBLIC ${imgui_external_SOURCE_DIR} lib src)

target_link_libraries(VoxelApp PRIVATE GLEW::GLEW glfw glm::glm OpenGL::GL imgui::imgui spdlog::spdlog VoxelEngine)

if(WIN32)
    add_custom_command(TARGET VoxelApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:VoxelApp>
            $<TARGET_FILE_DIR:VoxelApp>
        COMMAND_EXPAND_LISTS
    )
endif()

if(NOT EXISTS $<TARGET_FILE_DIR:VoxelApp>/assets)
    add_custom_command(TARGET VoxelApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:VoxelApp>/assets
    )
endif()