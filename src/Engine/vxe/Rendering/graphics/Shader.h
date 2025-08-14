#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace vxe {
    class Shader {
        public:
            virtual ~Shader() = default;

            virtual void bind() const = 0;
            virtual void unbind() const = 0;

            virtual void vertex(const std::string& str, const bool isSrc = false) = 0;
            virtual void fragment(const std::string& str, const bool isSrc = false) = 0;

            virtual void compile() = 0;

            virtual void setUniform(const std::string& name, const int value) = 0;
            virtual void setUniform(const std::string& name, const unsigned int value) = 0;
            virtual void setUniform(const std::string& name, const float value) = 0;
            virtual void setUniform(const std::string& name, const glm::vec2 value) = 0;
            virtual void setUniform(const std::string& name, const glm::vec3 value) = 0;
            virtual void setUniform(const std::string& name, const glm::vec4 value) = 0;
            virtual void setUniform(const std::string& name, const glm::ivec2 value) = 0;
            virtual void setUniform(const std::string& name, const glm::ivec3 value) = 0;
            virtual void setUniform(const std::string& name, const glm::mat4 value) = 0;

            static std::unique_ptr<Shader> create();
    };
}

#endif