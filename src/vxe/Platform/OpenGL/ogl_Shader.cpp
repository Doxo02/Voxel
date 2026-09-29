#include "ogl_Shader.h"

#include <GL/glew.h>
#include <fstream>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>

namespace vxe {
        OGLShader::OGLShader() {
            m_program = glCreateProgram();
        }

        OGLShader::~OGLShader() {
            glDeleteProgram(m_program);
        }

        void OGLShader::bind() const {
            glUseProgram(m_program);
        }
        void OGLShader::unbind() const {
            glUseProgram(0);
        }

        void OGLShader::vertex(const std::string& str, const bool isSrc) {
            std::string src;
            if (isSrc) {
                src = str;
            } else {
                std::ifstream vertexFile(str);
                if (!vertexFile.is_open()) {
                    spdlog::error("Failed to open vertex shader file: {0}", str);
                    throw std::runtime_error("Failed to open vertex shader file.");
                }
                src = std::string((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());
            }
            m_vert = glCreateShader(GL_VERTEX_SHADER);
            const char* vertCode = src.c_str();
            glShaderSource(m_vert, 1, &vertCode, NULL);
        }

        void OGLShader::fragment(const std::string& str, const bool isSrc) {
            std::string src;
            if (isSrc) {
                src = str;
            } else {
                std::ifstream fragmentFile(str);
                if (!fragmentFile.is_open()) {
                    spdlog::error("Failed to open fragment shader file: {0}", str);
                    throw std::runtime_error("Failed to open fragment shader file.");
                }
                src = std::string((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());
            }
            m_frag = glCreateShader(GL_FRAGMENT_SHADER);
            const char* fragCode = src.c_str();
            glShaderSource(m_frag, 1, &fragCode, NULL);
        }

        void OGLShader::compile() {
            glCompileShader(m_vert);
            checkCompileErrors(m_vert, "VERTEX");
            glCompileShader(m_frag);
            checkCompileErrors(m_frag, "FRAGMENT");

            glAttachShader(m_program, m_vert);
            glAttachShader(m_program, m_frag);
            glLinkProgram(m_program);
            checkCompileErrors(m_program, "PROGRAM");

            glDeleteShader(m_vert);
            glDeleteShader(m_frag);
        }

        void OGLShader::setUniform(const std::string& name, const int value) {
            glUniform1i(glGetUniformLocation(m_program, name.c_str()), value);
        }
        void OGLShader::setUniform(const std::string& name, const unsigned int value) {
            glUniform1ui(glGetUniformLocation(m_program, name.c_str()), value);
        }
        void OGLShader::setUniform(const std::string& name, const float value) {
            glUniform1f(glGetUniformLocation(m_program, name.c_str()), value);
        }
        void OGLShader::setUniform(const std::string& name, const glm::vec2 value) {
            glUniform2f(glGetUniformLocation(m_program, name.c_str()), value.x, value.y);
        }
        void OGLShader::setUniform(const std::string& name, const glm::vec3 value) {
            glUniform3f(glGetUniformLocation(m_program, name.c_str()), value.x, value.y, value.z);
        }
        void OGLShader::setUniform(const std::string& name, const glm::vec4 value) {
            glUniform4f(glGetUniformLocation(m_program, name.c_str()), value.x, value.y, value.z, value.w);
        }
        void OGLShader::setUniform(const std::string& name, const glm::ivec2 value) {
            glUniform2i(glGetUniformLocation(m_program, name.c_str()), value.x, value.y);
        }
        void OGLShader::setUniform(const std::string& name, const glm::ivec3 value) {
            glUniform3i(glGetUniformLocation(m_program, name.c_str()), value.x, value.y, value.z);
        }
        void OGLShader::setUniform(const std::string& name, const glm::mat4 value) {
            glUniformMatrix4fv(glGetUniformLocation(m_program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
        }

        void OGLShader::checkCompileErrors(GLuint shader, std::string type) {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    spdlog::error("Failed to compile {0} shader:\n\t{1}", type, infoLog);
                    throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
                }
            } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    spdlog::error("Failed to link Program:\n\t{0}", infoLog);
                    throw std::runtime_error("Program linking failed: " + std::string(infoLog));
                }
            }
        }
}