#include "ShaderStorageBuffer.h"

gla::ShaderStorageBuffer::ShaderStorageBuffer(void *data, unsigned int size, unsigned int index) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_id);
    m_index = index;
}

gla::ShaderStorageBuffer::~ShaderStorageBuffer() {
    glDeleteBuffers(1, &m_id);
}

void gla::ShaderStorageBuffer::bind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
}

void gla::ShaderStorageBuffer::bindBase() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_index, m_id);
}

void gla::ShaderStorageBuffer::unbind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
