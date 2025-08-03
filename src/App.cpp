#include "App.h"

#include <filesystem>

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

#include "platform/rss.h"
#include <FastNoiseLite.h>

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
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    glfwSetWindowUserPointer(m_window, this);

    // Set Callbacks
    glfwSetFramebufferSizeCallback(m_window, onResize);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);

    if(glewInit() != GLEW_OK){
        spdlog::critical("Failed to initialize GLEW!");
        return false;
    }
    spdlog::info("Initialized GLEW.");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, m_width, m_height);

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard | ImGuiConfigFlags_NavNoCaptureKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
    spdlog::info("Initialized ImGui");

    std::filesystem::path shaderDir = std::filesystem::current_path() / "assets" / "shader";
    m_program = new gla::Program((shaderDir / "raymarch.vert").string().c_str(), (shaderDir / "raymarch.frag").string().c_str(), true);
    m_program->bind();

    m_camera = new Camera(glm::vec3(50.0f, 20.0f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    m_projection = glm::perspective(glm::radians(m_camera->zoom), (float) m_width / (float) m_height, 0.1f, 100.0f);

    glm::ivec3 gridSize(64, 32, 64);
    m_world = new World(gridSize);

    spdlog::info("Starting to generate terrain...");
    
    double startTime = glfwGetTime();

    for (int z = 0; z < gridSize.z; z++) {
        for (int y = 0; y < gridSize.y; y++) {
            for (int x = 0; x < gridSize.x; x++) {
                m_world->generateBrick(glm::ivec3(x, y, z));
            }
        }
    }

    double took = glfwGetTime() - startTime;

    spdlog::info("Finished terrain generation. (size: {:.2f} MiB) (time taken: {:.2f}s)", m_world->getMap().getSizeInBytes() / 1024.0 / 1024.0, took);

    GPUBrickMap gpuBrickMap = m_world->getMap().getGPUMap();

    m_brickMapSSBO = new gla::ShaderStorageBuffer(gpuBrickMap.indexData.data(), gpuBrickMap.indexData.size() * sizeof(uint32_t), 0);
    m_brickSSBO = new gla::ShaderStorageBuffer(gpuBrickMap.bricks.data(), gpuBrickMap.bricks.size() * sizeof(GPUBrick), 1);
    m_colorSSBO = new gla::ShaderStorageBuffer(gpuBrickMap.colors.data(), gpuBrickMap.colors.size() * sizeof(glm::vec4), 2);

    glm::mat4 invVP = glm::inverse(m_projection * m_camera->getViewMatrix());
    m_program->setUniformMat4f("invViewProj", glm::value_ptr(invVP));
    m_program->setUniform3f("cameraPos", m_camera->position);
    m_program->setUniform2f("resolution", m_width, m_height);

    m_program->setUniform1ui("brickSize", BRICK_SIZE);
    m_program->setUniform3i("gridSize", gridSize);
    m_program->setUniform1f("voxelScale", 1.0f);

    m_dummyVAO = new gla::VertexArray();

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
    delete m_world;
    delete m_brickMapSSBO;
    delete m_brickSSBO;
    delete m_colorSSBO;
    delete m_dummyVAO;

    // Destroy GLFW window and terminate
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void App::run() {
    float voxelScale = 1.0;

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
        ImGui::End();

        if (viewportResized) {
            m_program->setUniform2f("resolution", m_width, m_height);
            viewportResized = false;
        }

        glm::mat4 invVP = glm::inverse(m_projection * m_camera->getViewMatrix());
        m_program->setUniform1f("time", glfwGetTime());
        m_program->setUniformMat4f("invViewProj", glm::value_ptr(invVP));
        m_program->setUniform3f("cameraPos", m_camera->position);
        m_program->setUniform1f("voxelScale", voxelScale);

        processInput(m_window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_program->bind();
        m_dummyVAO->bind();
        m_brickMapSSBO->bindBase();
        m_brickSSBO->bindBase();
        m_colorSSBO->bindBase();
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
