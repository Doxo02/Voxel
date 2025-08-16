#include "ogl_ShaderStorageBuffer.h"

vxe::OGLShaderStorageBuffer::OGLShaderStorageBuffer(unsigned int index) {
    glGenBuffers(1, &m_id);
    m_index = index;
}

vxe::OGLShaderStorageBuffer::~OGLShaderStorageBuffer() {
    glDeleteBuffers(1, &m_id);
}

void vxe::OGLShaderStorageBuffer::bind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
}

void vxe::OGLShaderStorageBuffer::bindBase() const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_index, m_id);
}

void vxe::OGLShaderStorageBuffer::unbind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void vxe::OGLShaderStorageBuffer::setData(void *data, unsigned int size) {
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_index, m_id);
    bindBase();
}