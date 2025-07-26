#include "VertexArray.h"

namespace gla {
    VertexArray::VertexArray() {
        glGenVertexArrays(1, &m_id);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_id);
    }

    void VertexArray::bind() {
        glBindVertexArray(m_id);
    }

    void VertexArray::unbind() {
        glBindVertexArray(0);
    }

    void VertexArray::setAttribute(unsigned int index, unsigned int size, unsigned int type, bool normalized, unsigned int stride, void* pointer) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }
} // namespace gla