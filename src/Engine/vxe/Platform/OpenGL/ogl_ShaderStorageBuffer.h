#ifndef OPENGL_SHADER_STORAGE_BUFFER
#define OPENGL_SHADER_STORAGE_BUFFER

#include "../../Rendering/graphics/ShaderStorageBuffer.h"

#include <GL/glew.h>

namespace vxe {
    class OGLShaderStorageBuffer : public ShaderStorageBuffer {
        public:
            OGLShaderStorageBuffer(void *data, unsigned int size, unsigned int index);
            ~OGLShaderStorageBuffer();

            OGLShaderStorageBuffer(const OGLShaderStorageBuffer&) = delete;
            OGLShaderStorageBuffer& operator=(const OGLShaderStorageBuffer&) = delete;

            void bind() const override;
            void bindBase() const override;
            void unbind() const override;
        private:
            GLuint m_id, m_index;
    };
}

#endif