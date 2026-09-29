#include "ogl_VertexArray.h"

namespace vxe {
    OGLVertexArray::OGLVertexArray() {
        glGenVertexArrays(1, &m_id);
        m_indexBuffer = nullptr;
    }

    OGLVertexArray::~OGLVertexArray() {
        glDeleteVertexArrays(1, &m_id);
    }

    void OGLVertexArray::bind() const {
        glBindVertexArray(m_id);
        if (m_indexBuffer != nullptr) {
            m_indexBuffer->bind();
        }
    }

    void OGLVertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void OGLVertexArray::addVertexBuffer(VertexBuffer* vertexBuffer, VertexAttribLayout* layouts, size_t size) {
        glBindVertexArray(m_id);
        vertexBuffer->setLayout(layouts, size);
        m_vertexBuffers.push_back(vertexBuffer);
    }

    void OGLVertexArray::setIndexBuffer(IndexBuffer* indexBuffer) {
        m_indexBuffer = indexBuffer;
    }
}