#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GL_Abstract/Program.h"
#include "GL_Abstract/VertexArray.h"
#include "GL_Abstract/VertexBuffer.h"
#include "GL_Abstract/ElementBuffer.h"

#include "Rendering/Camera.h"

#include "DataStructures/BrickMap.h"

#include <fstream>
#include <iostream>

#define WIDTH 800
#define HEIGHT 600

const int brickGridSizeX = 64;
const int brickGridSizeY = 32;
const int brickGridSizeZ = 64;

// Total number of possible bricks
const int totalBrickCells = brickGridSizeX * brickGridSizeY * brickGridSizeZ;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
glm::mat4 projection;

bool viewportResized = false;
bool cursorEnabled = false;

// GLFW window Resize callback
void onResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(camera.zoom), (float) width / (float) height, 0.1f, 100.0f);
    viewportResized = true;
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool pressingESC = false;

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

double lastX = 400;
double lastY = 400;
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

    if(glewInit() != GLEW_OK){
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);

    std::ifstream vertexFile("/home/lars/dev/Voxel/assets/shader/raymarch.vert");
    std::string vertexSource((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());
    std::ifstream fragmentFile("/home/lars/dev/Voxel/assets/shader/raymarch.frag");
    std::string fragmentSource((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());

    gla::Program program(vertexSource, fragmentSource);
    program.bind();

    projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

    struct GPUBrick {
        uint64_t bitmask[8];
        uint32_t colorOffset;
    };
    struct Color {
        float r, g, b, a;
    };

    std::vector<GPUBrick> bricks;
    std::vector<glm::ivec3> brickPositions;

    bricks.push_back(GPUBrick {
        {
            0x0000000000000001,
            0x0000000000000003,
            0x0000000000000007,
            0x000000000000000F,
            0x000000000000001F,
            0x000000000000003F,
            0x000000000000007F,
            0x00000000000000FF,
        },
         0
    });
    bricks.push_back(GPUBrick {
        {
            0x0000000000000000,
            0x000000FFFF000000,
            0x00000FFFFFF00000,
            0x0000FFFFFFFF0000,
            0x0000FFFFFFFF0000,
            0x00000FFFFFF00000,
            0x000000FFFF000000,
            0x0000000000000000,
        },
         0
    });
    brickPositions.push_back(glm::ivec3(0));
    brickPositions.push_back(glm::ivec3(3, 3, 3));

    std::vector<Color> voxelColors;

    for (int i = 0; i < 512; i++) {
        voxelColors.push_back(Color{1.0f, (float) i / 512.0f, 0.0f, 1.0f});
    }

    std::vector<uint32_t> brickMapIndexData(totalBrickCells, 0xFFFFFFFF);

    struct hash_ivec3 {
        size_t operator()(const glm::ivec3& v) const {
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y << 1) ^ std::hash<int>()(v.z << 2);
        }
    };

    std::unordered_map<glm::ivec3, int, hash_ivec3> brickIndexLookup;

    for (int i = 0; i < bricks.size(); i++) {
        glm::ivec3 brickPos = brickPositions[i];
        brickIndexLookup[brickPos] = i;

        int flatIndex = brickPos.x + brickPos.y * brickGridSizeX + brickPos.z * brickGridSizeX * brickGridSizeY;

        brickMapIndexData[flatIndex] = i;
    }

    GLuint brickMapTex;
    glGenTextures(1, &brickMapTex);
    glBindTexture(GL_TEXTURE_3D, brickMapTex);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, brickGridSizeX, brickGridSizeY, brickGridSizeZ);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                    brickGridSizeX, brickGridSizeY, brickGridSizeZ,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, brickMapIndexData.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    program.setUniform1i("brickMap", 0);

    GLuint brickSSBO;
    glGenBuffers(1, &brickSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, brickSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bricks.size() * sizeof(GPUBrick), bricks.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, brickSSBO);

    GLuint colorSSBO;
    glGenBuffers(1, &colorSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, voxelColors.size() * sizeof(Color), voxelColors.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, colorSSBO);

    glm::mat4 invVP = glm::inverse(projection * camera.getViewMatrix());
    program.setUniformMat4f("invViewProj", glm::value_ptr(invVP));
    program.setUniform3f("cameraPos", camera.position.x, camera.position.y, camera.position.z);
    program.setUniform2f("resolution", WIDTH, HEIGHT);

    GLuint dummyVAO;
    glGenVertexArrays(1, &dummyVAO);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (viewportResized) {
            program.setUniform2f("resolution", WIDTH, HEIGHT);
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

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
