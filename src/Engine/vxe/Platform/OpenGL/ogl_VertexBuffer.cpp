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

    void OGLVertexBuffer::setLayout(VertexAttribLayout* layouts, size_t size) const {
        // Calculate stride
        size_t stride = 0;
        for (size_t i = 0; i < size; i++) {
            switch(layouts[i].type) {
                case VertexAttribType::FLOAT:
                case VertexAttribType::INT:
                case VertexAttribType::UNSIGNED_INT:
                    stride += layouts[i].size * 4;
                    break;
                case VertexAttribType::DOUBLE:
                    stride += layouts[i].size * 8;
                    break;
                case VertexAttribType::BYTE:
                case VertexAttribType::UNSIGNED_BYTE:
                    stride += layouts[i].size;
                    break;
                case VertexAttribType::SHORT:
                case VertexAttribType::UNSIGNED_SHORT:
                    stride += layouts[i].size * 2;
                    break;
                    
            }
        }

        size_t currentOffset = 0;
        for (size_t i = 0; i < size; i++) {
            glEnableVertexAttribArray(layouts[i].index);
            switch(layouts[i].type) {
                case VertexAttribType::FLOAT:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_FLOAT, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 4;
                    break;
                case VertexAttribType::DOUBLE:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_DOUBLE, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 8;
                    break;
                case VertexAttribType::BYTE:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_BYTE, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size;
                    break;
                case VertexAttribType::UNSIGNED_BYTE:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_UNSIGNED_BYTE, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size;
                    break;
                case VertexAttribType::SHORT:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_SHORT, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 2;
                    break;
                case VertexAttribType::UNSIGNED_SHORT:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_UNSIGNED_SHORT, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 2;
                    break;
                case VertexAttribType::INT:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_INT, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 4;
                    break;
                case VertexAttribType::UNSIGNED_INT:
                    glVertexAttribPointer(layouts[i].index, layouts[i].size, GL_UNSIGNED_INT, GL_FALSE, stride, (void*) currentOffset);
                    currentOffset += layouts[i].size * 4;
                    break;
            }
        }
    }
}