#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

#include "GL_Abstract/Program.h"
#include "GL_Abstract/VertexArray.h"
#include "GL_Abstract/VertexBuffer.h"
#include "GL_Abstract/ElementBuffer.h"

#include <fstream>

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
};

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
};

std::vector<unsigned int> indices = {
    0, 1, 2
};

int main(int, char**){
    if(!glfwInit()){
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if(!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK){
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gla::VertexArray vao;
    vao.bind();
    gla::VertexBuffer vbo(vertices.data(), vertices.size() * sizeof(Vertex));
    gla::ElementBuffer ebo(indices.data(), indices.size() * sizeof(unsigned int));

    vao.setAttribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.setAttribute(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    std::ifstream vertexFile("./assets/shader/basic.vert");
    std::string vertexSource((std::istreambuf_iterator<char
        >(vertexFile)), std::istreambuf_iterator<char>());
    std::ifstream fragmentFile("./assets/shader/basic.frag");
    std::string fragmentSource((std::istreambuf_iterator<char
        >(fragmentFile)), std::istreambuf_iterator<char>());

    gla::Program program(vertexSource, fragmentSource);

    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);

        program.bind();
        vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
