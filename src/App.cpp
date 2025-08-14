#include "App.h"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

#include "platform/rss.h"
#include <FastNoiseLite.h>

#include "vxe/DataStructures/BrickMap.h"

std::vector<vxe::MaterialInfo> materialInfos = {
    { glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0.0f },     // AIR
    { glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, 0.2f },     // GRASS
    { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), 0.2f, 0.7f }      // STONE
};

App::App(int width, int height, const char* title)
    : m_width(width), m_height(height), m_title(title) {}

bool App::init() {
    if (!glfwInit()) {
        spdlog::critical("Failed to initialize GLFW!");
        return false;
    }
    spdlog::info("Initialized GLFW.");

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
    if (!m_window)
    {
        spdlog::critical("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    spdlog::info("Created window.");

    glfwMakeContextCurrent(m_window);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize cursor state to match GLFW setting
    cursorEnabled = false;

    glfwSetWindowUserPointer(m_window, this);

    // Set Callbacks
    glfwSetFramebufferSizeCallback(m_window, onResize);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);

    m_renderer = new vxe::Renderer();
    m_renderer->init();
    m_renderer->getAPI()->setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_renderer->getAPI()->setViewport(0, 0, m_width, m_height);

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
    spdlog::info("Initialized ImGui");

    std::filesystem::path shaderDir = std::filesystem::current_path() / "assets" / "shader";
    m_program = vxe::Shader::create();
    m_program->vertex((shaderDir / "raymarch.vert").string());
    m_program->fragment((shaderDir / "raymarch.frag").string());
    m_program->compile();
    m_program->bind();

    m_camera = new Camera(glm::vec3(80.0f, 70.0f, 70.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    m_projection = glm::perspective(glm::radians(m_camera->zoom), (float) m_width / (float) m_height, 0.1f, 100.0f);

    glm::ivec3 gridSize(64, 32, 64);

    m_grid = new vxe::VoxelGrid(vxe::GridType::BRICK_MAP, gridSize);

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

    // Clean up dynamically allocated objects
    delete m_program;
    delete m_camera;
    delete m_brickMapSSBO;
    delete m_brickSSBO;
    delete m_materialSSBO;
    delete m_materialInfosSSBO;
    delete m_grid;
    delete m_renderer;

    // Destroy GLFW window and terminate
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void App::run() {
    float voxelScale = 1.0f;

    glm::vec3 lightPos(80.0f, 70.0f, 80.0f);
    glm::vec3 lightColor(1.0f);
    float lightIntensity = 1.0f;

    while (!glfwWindowShouldClose(m_window)) {
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

        processInput(m_window);

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
        m_renderer->submit(m_grid);

        // error = glGetError();
        // if (error != GL_NO_ERROR) {
        //     spdlog::error("OpenGL error: {}", error);
        // }

        m_renderer->endFrame();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);

        glfwPollEvents();
    }
}

void App::onResize(GLFWwindow *window, int width, int height) {
    App* userPointer = (App*) glfwGetWindowUserPointer(window);
    glViewport(0, 0, width, height);
    userPointer->m_projection = glm::perspective(glm::radians(userPointer->m_camera->zoom), (float) width / (float) height, 0.1f, 100.0f);
    userPointer->viewportResized = true;

    userPointer->m_width = width;
    userPointer->m_height = height;
}

void App::processInput(GLFWwindow *window) {
    App* userPointer = (App*) glfwGetWindowUserPointer(window);

    if (glfwGetKey(window, GLFW_KEY_W))
        userPointer->m_camera->processKeyboard(Camera::FORWARD, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A))
        userPointer->m_camera->processKeyboard(Camera::LEFT, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S))
        userPointer->m_camera->processKeyboard(Camera::BACKWARD, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D))
        userPointer->m_camera->processKeyboard(Camera::RIGHT, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE))
        userPointer->m_camera->processKeyboard(Camera::UP, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        userPointer->m_camera->processKeyboard(Camera::DOWN, userPointer->deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !userPointer->pressingESC) {
        userPointer->pressingESC = true;
        if (userPointer->cursorEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        userPointer->cursorEnabled = !userPointer->cursorEnabled;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        userPointer->pressingESC = false;
}

void App::mouseCallback(GLFWwindow *window, double xPos, double yPos) {
    App* userPointer = (App*) glfwGetWindowUserPointer(window);

    if (userPointer->cursorEnabled) return;
    double xOffset = userPointer->lastX - xPos;
    double yOffset = userPointer->lastY - yPos;
    userPointer->lastX = xPos;
    userPointer->lastY = yPos;

    userPointer->m_camera->processMouseMovement(xOffset, yOffset);
}

void App::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    App* userPointer = (App*) glfwGetWindowUserPointer(window);
    userPointer->m_camera->processMouseScroll(yoffset);
}
