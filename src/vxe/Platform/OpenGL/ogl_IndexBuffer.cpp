#include "ogl_IndexBuffer.h"

namespace vxe {
    OGLIndexBuffer::OGLIndexBuffer(unsigned int* indices, size_t count) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    }

    OGLIndexBuffer::~OGLIndexBuffer() {
        glDeleteBuffers(1, &m_id);
    }

    void OGLIndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    }

    void OGLIndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}