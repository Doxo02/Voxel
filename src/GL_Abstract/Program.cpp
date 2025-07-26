#include "Program.h"

#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include <spdlog/spdlog.h>

namespace gla {

void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            spdlog::error("Failed to compile {0} shader:\n\t{1}", type, infoLog);    
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            spdlog::error("Failed to link Program:\n\t{0}", infoLog);
        }
    }
}

Program::Program(const std::string& vertexShader, const std::string& fragmentShader, bool isPath) {
    std::string vertexSource, fragmentSource;
    if (isPath) {
        std::ifstream vertexFile(vertexShader);
        if (!vertexFile.is_open()) {
            spdlog::error("Failed to open vertex shader file: {0}", vertexShader);
            throw std::runtime_error("Failed to open vertex shader file.");
        }
        vertexSource = std::string((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());

        std::ifstream fragmentFile(fragmentShader);
        if (!fragmentFile.is_open()) {
            spdlog::error("Failed to open fragment shader file: {0}", fragmentShader);
            throw std::runtime_error("Failed to open fragment shader file.");
        }
        fragmentSource = std::string((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());
    } else {
        vertexSource = vertexShader;
        fragmentSource = fragmentShader;
    }

    GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexCode = vertexSource.c_str();
    glShaderSource(vertexId, 1, &vertexCode, NULL);
    glCompileShader(vertexId);
    checkCompileErrors(vertexId, "VERTEX");

    GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentCode = fragmentSource.c_str();
    glShaderSource(fragmentId, 1, &fragmentCode, NULL);
    glCompileShader(fragmentId);
    checkCompileErrors(fragmentId, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexId);
    glAttachShader(m_id, fragmentId);
    glLinkProgram(m_id);
    checkCompileErrors(m_id, "PROGRAM");

    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);
}

Program::~Program() {
    glDeleteProgram(m_id);
}

void Program::bind() {
    glUseProgram(m_id);
}

void Program::unbind() {
    glUseProgram(0);
}

unsigned int Program::compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    return id;
}

void Program::setUniform1i(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Program::setUniform3i(const std::string &name, glm::ivec3 value) {
    glUniform3i(glGetUniformLocation(m_id, name.c_str()), value.x, value.y, value.z);
}

void Program::setUniform1ui(const std::string& name, unsigned int value) {
    glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
}

void Program::setUniform1f(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Program::setUniform2f(const std::string& name, float v0, float v1) {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), v0, v1);
}

void Program::setUniform3f(const std::string& name, float v0, float v1, float v2) {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), v0, v1, v2);
}

void Program::setUniform3f(const std::string &name, glm::vec3 value) {
    setUniform3f(name, value.x, value.y, value.z);
}

void Program::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), v0, v1, v2, v3);
}

void Program::setUniformMat4f(const std::string& name, const GLfloat* value) {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, value);
}

} // namespace gla