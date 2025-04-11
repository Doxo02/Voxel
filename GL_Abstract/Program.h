#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

namespace gla {
    
    class Program {
    public:
        Program(const std::string& vertexShader, const std::string& fragmentShader);
        ~Program();

        void bind();
        void unbind();

        void setUniform1i(const std::string& name, int value);
        void setUniform3i(const std::string& name, glm::ivec3 value);
        void setUniform1ui(const std::string& name, unsigned int value);
        void setUniform1f(const std::string& name, float value);
        void setUniform2f(const std::string& name, float v0, float v1);
        void setUniform3f(const std::string& name, float v0, float v1, float v2);
        void setUniform3f(const std::string& name, glm::vec3 value);
        void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
        void setUniformMat4f(const std::string& name, const GLfloat* matrix);

    private:
        unsigned int m_id;

        unsigned int compileShader(unsigned int type, const std::string& source);
    };
}

#endif // PROGRAM_H