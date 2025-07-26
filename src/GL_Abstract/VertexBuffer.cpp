#include "VertexBuffer.h"

namespace gla {
    VertexBuffer::VertexBuffer(void* data, unsigned int size) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_id);
    }

    void VertexBuffer::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
    }

    void VertexBuffer::unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
} // namespace gla