#include "ElementBuffer.h"

namespace gla {
    
    ElementBuffer::ElementBuffer(void* data, unsigned int size) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    ElementBuffer::~ElementBuffer() {
        glDeleteBuffers(1, &m_id);
    }

    void ElementBuffer::bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    }

    void ElementBuffer::unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
} // namespace gla