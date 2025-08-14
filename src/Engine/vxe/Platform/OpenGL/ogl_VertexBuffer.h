#ifndef OPENGL_VERTEX_BUFFER_H
#define OPENGL_VERTEX_BUFFER_H

#include "../../Rendering/graphics/VertexBuffer.h"

#include <GL/glew.h>

namespace vxe {
    class OGLVertexBuffer : public VertexBuffer {
        public:
            OGLVertexBuffer(const void* data, size_t size);
            ~OGLVertexBuffer();

            void bind() const override;
            void unbind() const override;
        
        private:
            GLuint m_id;
    };
}

#endif