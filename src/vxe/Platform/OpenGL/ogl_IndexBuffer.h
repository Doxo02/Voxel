#ifndef OPENGL_INDEX_BUFFER_H
#define OPENGL_INDEX_BUFFER_H

#include <cstddef>
#include <GL/glew.h>

#include "../../Rendering/graphics/IndexBuffer.h"

namespace vxe {
    class OGLIndexBuffer : public IndexBuffer {
        public:
            OGLIndexBuffer(unsigned int* indices, size_t count);
            ~OGLIndexBuffer();

            void bind() const override;
            void unbind() const override;
        
        private:
            GLuint m_id;
    };
}

#endif