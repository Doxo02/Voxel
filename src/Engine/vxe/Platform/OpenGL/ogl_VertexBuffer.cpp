#include "ogl_VertexBuffer.h"

namespace vxe {
    OGLVertexBuffer::OGLVertexBuffer(const void* data, size_t size) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferData(m_id, size, data, GL_STATIC_DRAW);
    }

    OGLVertexBuffer::~OGLVertexBuffer() {
        glDeleteBuffers(1, &m_id);
    }

    void OGLVertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
    }

    void OGLVertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}