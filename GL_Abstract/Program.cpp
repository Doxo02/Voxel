#include "Program.h"

#include <iostream>

namespace gla {
    
    Program::Program(const std::string& vertexShader, const std::string& fragmentShader) {
        m_id = glCreateProgram();
        unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
        glAttachShader(m_id, vs);
        glAttachShader(m_id, fs);
        glLinkProgram(m_id);
        glValidateProgram(m_id);
        glDeleteShader(vs);
        glDeleteShader(fs);
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
} // namespace gla