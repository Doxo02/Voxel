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

#include <fstream>
#include <iostream>

#define WIDTH 800
#define HEIGHT 600

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
};

std::vector<Vertex> vertices = {
    {{-1, -1, -1}, {1.0f, 0.0f, 0.0f, 1.0f}},      // front bottom-left
    {{1, -1, -1}, {0.0f, 1.0f, 0.0f, 1.0f}},       // front top-left
    {{1, 1, -1}, {0.0f, 0.0f, 1.0f, 1.0f}},        // front top-right
    {{-1, 1, -1}, {1.0f, 1.0f, 0.0f, 1.0f}},       // front bottom-right
    {{-1, -1, 1}, {1.0f, 0.0f, 1.0f, 1.0f}},       // back bottom-left
    {{1, -1, 1}, {0.0f, 1.0f, 1.0f, 1.0f}},        // back top-left
    {{1, 1, 1}, {1.0f, 1.0f, 1.0f, 1.0f}},         // back top-right
    {{-1, 1, 1}, {1.0f, 0.0f, 0.0f, 1.0f}}         // back bottom-right
};

std::vector<unsigned int> indices = {
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
};

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
glm::mat4 projection;

bool viewportResized = false;

// GLFW window Resize callback
void onResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(camera.zoom), (float) width / (float) height, 0.1f, 100.0f);
    viewportResized = true;
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

double lastX = 400;
double lastY = 400;
void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    double xOffset = lastX - xPos;
    double yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    camera.processMouseMovement(xOffset, yOffset);
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

    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK){
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);

    gla::VertexArray vao;
    vao.bind();
    gla::VertexBuffer vbo(vertices.data(), vertices.size() * sizeof(Vertex));
    gla::ElementBuffer ebo(indices.data(), indices.size() * sizeof(unsigned int));

    vao.setAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.setAttribute(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    std::ifstream vertexFile("./assets/shader/basic.vert");
    std::string vertexSource((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());
    std::ifstream fragmentFile("./assets/shader/basic.frag");
    std::string fragmentSource((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());

    gla::Program program(vertexSource, fragmentSource);
    program.bind();
    program.setUniformMat4f("view", glm::value_ptr(camera.getViewMatrix()));

    projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
    program.setUniformMat4f("projection", glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    program.setUniformMat4f("model", glm::value_ptr(model));

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (viewportResized) {
            program.setUniformMat4f("projection", glm::value_ptr(projection));
            viewportResized = false;
        }

        processInput(window);
        program.setUniformMat4f("view", glm::value_ptr(camera.getViewMatrix()));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.bind();
        vao.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
