#include "App.h"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

#include "platform/rss.h"
#include <FastNoiseLite.h>
#include <GLFW/glfw3.h>

#include "vxe/DataStructures/BrickMap.h"

std::vector<vxe::MaterialInfo> materialInfos = {
    { glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0.0f },     // AIR
    { glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, 0.2f },     // GRASS
    { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), 0.2f, 0.7f }      // STONE
};

App::App(int width, int height, const char* title)
    : m_width(width), m_height(height), m_title(title) {}

bool App::init() {
    m_window = vxe::Window::Create({m_title, (uint32_t) m_width, (uint32_t) m_height});

    m_renderer = std::make_unique<vxe::Renderer>();
    m_renderer->init(m_window.get());
    m_renderer->getAPI()->setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_renderer->getAPI()->setViewport(0, 0, m_width, m_height);

    m_window->init();
    m_window->setCursorEnabled(false);

    VXE_SUBSCRIBE_MEMBER(vxe::WindowResizeEvent, this, &App::onResize);
    VXE_SUBSCRIBE_MEMBER(vxe::KeyPressedEvent, this, &App::onKeyPressed);
    VXE_SUBSCRIBE_MEMBER(vxe::MouseMovedEvent, this, &App::onMouseMove);
    VXE_SUBSCRIBE_MEMBER(vxe::MouseScrolledEvent, this, &App::onMouseScroll);
    VXE_SUBSCRIBE_MEMBER(vxe::WindowCloseEvent, this, &App::onWindowClose);

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_window->getNativeWindow()), true);
    ImGui_ImplOpenGL3_Init();
    spdlog::info("Initialized ImGui");

    std::filesystem::path shaderDir = std::filesystem::current_path() / "assets" / "shader";
    m_program = vxe::Shader::create();
    m_program->vertex((shaderDir / "raymarch.vert").string());
    m_program->fragment((shaderDir / "raymarch.frag").string());
    m_program->compile();
    m_program->bind();

    m_camera = std::make_unique<Camera>(glm::vec3(80.0f, 70.0f, 70.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    m_projection = glm::perspective(glm::radians(m_camera->zoom), (float) m_width / (float) m_height, 0.1f, 100.0f);

    glm::ivec3 gridSize(64, 32, 64);

    m_grid = std::make_unique<vxe::VoxelGrid>(vxe::GridType::BRICK_MAP, gridSize);

    spdlog::info("Starting to generate terrain...");
    
    double startTime = glfwGetTime();

    for (int z = 0; z < gridSize.z; z++) {
        for (int y = 0; y < gridSize.y; y++) {
            for (int x = 0; x < gridSize.x; x++) {
                m_grid->getGrid()->generateChunk(glm::ivec3(x, y, z));
            }
        }
    }

    double took = glfwGetTime() - startTime;

    spdlog::info("Finished terrain generation. (size: {:.2f} MiB) (time taken: {:.2f}s)", m_grid->getGrid()->getSizeInBytes() / 1024.0 / 1024.0, took);

    vxe::GPUGrid gpuBrickMap = m_grid->getGrid()->getGPUGrid();

    // m_grid->bindVA();
    m_brickMapSSBO = vxe::ShaderStorageBuffer::create(gpuBrickMap.indexData.data(), gpuBrickMap.indexData.size() * sizeof(uint32_t), 0);
    m_brickSSBO = vxe::ShaderStorageBuffer::create(gpuBrickMap.bricks.data(), gpuBrickMap.bricks.size() * sizeof(vxe::GPUBrick), 1);
    m_materialSSBO = vxe::ShaderStorageBuffer::create(gpuBrickMap.materials.data(), gpuBrickMap.materials.size() * sizeof(uint32_t), 2);
    m_materialInfosSSBO = vxe::ShaderStorageBuffer::create(materialInfos.data(), materialInfos.size() * sizeof(vxe::MaterialInfo), 3);

    glm::mat4 invVP = glm::inverse(m_projection * m_camera->getViewMatrix());
    m_program->setUniform("invViewProj", invVP);
    m_program->setUniform("cameraPos", m_camera->position);
    m_program->setUniform("resolution", glm::vec2(m_width, m_height));

    m_program->setUniform("brickSize", (unsigned int) BRICK_SIZE);
    m_program->setUniform("gridSize", gridSize);
    m_program->setUniform("voxelScale", 1.0f);

    return true;
}

void App::terminate() {
    // Clean up ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Smart pointers will automatically clean up all allocated objects
    // No manual delete calls needed
}

void App::run() {
    float voxelScale = 1.0f;

    glm::vec3 lightPos(80.0f, 70.0f, 80.0f);
    glm::vec3 lightColor(1.0f);
    float lightIntensity = 1.0f;

    while (running) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Info");
        ImGui::Text("%.4f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::Text("Camere pos: (%.2f, %.2f, %.2f)", m_camera->position.x, m_camera->position.y, m_camera->position.z);
        ImGui::Text("Memory (MiB): %.4f", (float) getCurrentRSS() / (1024.0 * 1024.0));
        ImGui::SliderFloat("Voxel Scale", &voxelScale, 0.0, 2.0);
        ImGui::InputFloat3("Light Pos", glm::value_ptr(lightPos));
        ImGui::InputFloat3("Light Color", glm::value_ptr(lightColor));
        ImGui::InputFloat("Light Intensity", &lightIntensity, 0.01, 0.1);
        ImGui::End();

        if (viewportResized) {
            m_program->setUniform("resolution", glm::vec2(m_width, m_height));
            viewportResized = false;
        }

        glm::mat4 invVP = glm::inverse(m_projection * m_camera->getViewMatrix());
        m_program->setUniform("time", (float) glfwGetTime());
        m_program->setUniform("invViewProj", invVP);
        m_program->setUniform("cameraPos", m_camera->position);
        m_program->setUniform("voxelScale", voxelScale);
        m_program->setUniform("lightPos", lightPos);
        m_program->setUniform("lightColor", lightColor);
        m_program->setUniform("lightIntensity", lightIntensity);

        processInput();

        m_renderer->beginFrame();

        // GLenum error = glGetError();
        // if (error != GL_NO_ERROR) {
        //     spdlog::error("OpenGL error: {}", error);
        // }

        m_program->bind();
        m_brickMapSSBO->bindBase();
        m_brickSSBO->bindBase();
        m_materialSSBO->bindBase();
        m_materialInfosSSBO->bindBase();
        m_renderer->submit(m_grid.get());

        // error = glGetError();
        // if (error != GL_NO_ERROR) {
        //     spdlog::error("OpenGL error: {}", error);
        // }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_renderer->endFrame();
        m_window->onUpdate();
    }
}

void App::processInput() {
    if (m_window->isKeyDown(vxe::Key::W))
        m_camera->processKeyboard(Camera::FORWARD, deltaTime);
    if (m_window->isKeyDown(vxe::Key::S))
        m_camera->processKeyboard(Camera::BACKWARD, deltaTime);
    if (m_window->isKeyDown(vxe::Key::A))
        m_camera->processKeyboard(Camera::LEFT, deltaTime);
    if (m_window->isKeyDown(vxe::Key::D))
        m_camera->processKeyboard(Camera::RIGHT, deltaTime);
    if (m_window->isKeyDown(vxe::Key::Space))
        m_camera->processKeyboard(Camera::UP, deltaTime);
    if (m_window->isKeyDown(vxe::Key::LeftShift))
        m_camera->processKeyboard(Camera::DOWN, deltaTime);
}

bool App::onResize(vxe::WindowResizeEvent& e) {
    m_renderer->getAPI()->setViewport(0, 0, e.getWidth(), e.getHeight());
    m_projection = glm::perspective(glm::radians(m_camera->zoom), (float) e.getWidth() / (float) e.getHeight(), 0.1f, 100.0f);
    viewportResized = true;

    m_width = e.getWidth();
    m_height = e.getHeight();

    return true;
}

bool App::onKeyPressed(vxe::KeyPressedEvent& e) {
    if (e.getKeyCode() == (int) vxe::Key::Escape) {
        if (cursorEnabled) {
            m_window->setCursorEnabled(false);
        } else {
            m_window->setCursorEnabled(true);
        }

        cursorEnabled = !cursorEnabled;
        return true;
    }
    return false;
}

bool App::onKeyReleased(vxe::KeyReleasedEvent& e) {
    return false;
}

bool App::onMouseMove(vxe::MouseMovedEvent& e) {

    if (cursorEnabled) return false;
    double xOffset = lastX - e.getX();
    double yOffset = lastY - e.getY();
    lastX = e.getX();
    lastY = e.getY();

    m_camera->processMouseMovement(xOffset, yOffset);
    return true;
}

bool App::onMouseScroll(vxe::MouseScrolledEvent& e) {
    m_camera->processMouseScroll(e.getYOffset());
    return true;
}

bool App::onWindowClose(vxe::WindowCloseEvent& e) {
    running = false;
    return true;
}


vxe::Application* vxe::createApplication() {
    App* app = new App(800, 600, "test");
    app->init();
    return app;
}