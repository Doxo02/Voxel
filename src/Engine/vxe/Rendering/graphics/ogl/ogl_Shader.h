#ifndef OPENGL_SHADER_H
#define OPENGL_SHADER_H

#include "../Shader.h"

#include <GL/glew.h>

namespace vxe {
    class OGLShader : public Shader {
        public:
            OGLShader();
            ~OGLShader();

            void bind() const override;
            void unbind() const override;

            void vertex(const std::string& str, const bool isSrc = false) override;
            void fragment(const std::string& str, const bool isSrc = false) override;

            void compile() override;

            void setUniform(const std::string& name, const int value) override;
            void setUniform(const std::string& name, const unsigned int value) override;
            void setUniform(const std::string& name, const float value) override;
            void setUniform(const std::string& name, const glm::vec2 value) override;
            void setUniform(const std::string& name, const glm::vec3 value) override;
            void setUniform(const std::string& name, const glm::vec4 value) override;
            void setUniform(const std::string& name, const glm::ivec2 value) override;
            void setUniform(const std::string& name, const glm::ivec3 value) override;
            void setUniform(const std::string& name, const glm::mat4 value) override;

        private:
            GLuint m_program, m_vert, m_frag;

            unsigned int compileShader(unsigned int type, const std::string& source);
            void checkCompileErrors(GLuint shader, std::string type);
    };
}

#endif