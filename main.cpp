#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include "GL_Abstract/Program.h"
#include "GL_Abstract/VertexArray.h"
#include "GL_Abstract/VertexBuffer.h"
#include "GL_Abstract/ElementBuffer.h"

#include "Rendering/Camera.h"

#include "DataStructures/BrickMap.h"

#include <fstream>
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define WIDTH 800
#define HEIGHT 600

const int brickGridSizeX = 64;
const int brickGridSizeY = 32;
const int brickGridSizeZ = 64;

const int totalBrickCells = brickGridSizeX * brickGridSizeY * brickGridSizeZ;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
glm::mat4 projection;

bool viewportResized = false;
glm::ivec2 curRes(WIDTH, HEIGHT);
bool cursorEnabled = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool pressingESC = false;

double lastX = 400;
double lastY = 400;

// GLFW window Resize callback
void onResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(camera.zoom), (float) width / (float) height, 0.1f, 100.0f);
    viewportResized = true;
    curRes = glm::ivec2(width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W))
        camera.processKeyboard(Camera::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A))
        camera.processKeyboard(Camera::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S))
        camera.processKeyboard(Camera::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D))
        camera.processKeyboard(Camera::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE))
        camera.processKeyboard(Camera::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        camera.processKeyboard(Camera::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !pressingESC) {
        pressingESC = true;
        if (cursorEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        cursorEnabled = !cursorEnabled;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        pressingESC = false;
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (cursorEnabled) return;
    double xOffset = lastX - xPos;
    double yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    camera.processMouseMovement(xOffset, yOffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(yoffset);
}

int main(int, char**){
    if(!glfwInit()){
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if(!window){
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, onResize);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if(glewInit() != GLEW_OK){
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    std::ifstream vertexFile("/home/lars/dev/Voxel/assets/shader/raymarch.vert");
    std::string vertexSource((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());
    std::ifstream fragmentFile("/home/lars/dev/Voxel/assets/shader/raymarch.frag");
    std::string fragmentSource((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());

    gla::Program program(vertexSource, fragmentSource);
    program.bind();

    projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

    BrickMap brickMap(glm::ivec3(brickGridSizeX, brickGridSizeY, brickGridSizeZ));

    const glm::vec3 sphereCenter = glm::vec3(40.0f, 20.0f, 40.0f);
    const float sphereRadius = 10.0f;
    const float radiusSquared = sphereRadius * sphereRadius;

    glm::ivec3 minVoxel = glm::floor(sphereCenter - glm::vec3(sphereRadius));
    glm::ivec3 maxVoxel = glm::ceil(sphereCenter + glm::vec3(sphereRadius));

    for (int z = minVoxel.z; z < maxVoxel.z; z++) {
        for (int y = minVoxel.y; y < maxVoxel.y; y++) {
            for (int x = minVoxel.x; x < maxVoxel.x; x++) {
                glm::vec3 voxelPos = glm::vec3(x, y, z);
                float dist2 = glm::distance2(voxelPos, sphereCenter);

                if (dist2 <= radiusSquared) {
                    brickMap.setVoxel(voxelPos, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                }
            }
        }
    }

    brickMap.setVoxel(glm::vec3(7, 7, 7), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    GPUBrickMap gpuBrickMap = brickMap.getGPUMap();

    GLuint brickMapTex;
    glGenTextures(1, &brickMapTex);
    glBindTexture(GL_TEXTURE_3D, brickMapTex);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, brickGridSizeX, brickGridSizeY, brickGridSizeZ);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                    brickGridSizeX, brickGridSizeY, brickGridSizeZ,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, gpuBrickMap.indexData.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    program.setUniform1i("brickMap", 0);

    GLuint brickSSBO;
    glGenBuffers(1, &brickSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, brickSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuBrickMap.bricks.size() * sizeof(GPUBrick), gpuBrickMap.bricks.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, brickSSBO);

    GLuint colorSSBO;
    glGenBuffers(1, &colorSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuBrickMap.colors.size() * sizeof(glm::vec4), gpuBrickMap.colors.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorSSBO);

    glm::mat4 invVP = glm::inverse(projection * camera.getViewMatrix());
    program.setUniformMat4f("invViewProj", glm::value_ptr(invVP));
    program.setUniform3f("cameraPos", camera.position.x, camera.position.y, camera.position.z);
    program.setUniform2f("resolution", WIDTH, HEIGHT);

    program.setUniform1ui("brickSize", BRICK_SIZE);
    program.setUniform3f("gridSize", brickGridSizeX, brickGridSizeY, brickGridSizeZ);

    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Info");
        ImGui::Text("%.4f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::End();

        if (viewportResized) {
            program.setUniform2f("resolution", curRes.x, curRes.y);
            viewportResized = false;
        }

        glm::mat4 invVP = glm::inverse(projection * camera.getViewMatrix());
        program.setUniformMat4f("invViewProj", glm::value_ptr(invVP));
        program.setUniform3f("cameraPos", camera.position.x, camera.position.y, camera.position.z);

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.bind();
        glBindVertexArray(dummyVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, brickMapTex);
        program.setUniform1i("brickMap", 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, brickSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorSSBO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
