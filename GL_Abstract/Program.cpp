#include "Program.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace gla {

void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

Program::Program(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexCode = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexCode, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentCode = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragmentShader);
    glLinkProgram(m_id);
    checkCompileErrors(m_id, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
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

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        // TODO: Log this
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Program::setUniform1i(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
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

void Program::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), v0, v1, v2, v3);
}

void Program::setUniformMat4f(const std::string& name, const GLfloat* value) {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, value);
}

} // namespace gla