#include "IndexBuffer.h"
#include "RenderAPI.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"


#include "ogl/ogl_IndexBuffer.h"
#include "ogl/ogl_RenderAPI.h"
#include "ogl/ogl_Shader.h"
#include "ogl/ogl_ShaderStorageBuffer.h"
#include "ogl/ogl_VertexArray.h"
#include "ogl/ogl_VertexBuffer.h"

namespace vxe {
    IndexBuffer* IndexBuffer::create(unsigned int* indices, size_t count) {
        // TODO: add config to select the API
        return new OGLIndexBuffer(indices, count);
    }

    RenderAPI* RenderAPI::create() {
        // TODO: add config to select the API
        return new OGLRenderAPI();
    }

    Shader* Shader::create() {
        // TODO: add config to select the API
        return new OGLShader();
    }

    ShaderStorageBuffer* ShaderStorageBuffer::create(void *data, unsigned int size, unsigned int index) {
        // TODO: add config to select the API
        return new OGLShaderStorageBuffer(data, size, index);
    }

    VertexArray* VertexArray::create() {
        // TODO: add config to select the API
        return new OGLVertexArray();
    }

    VertexBuffer* VertexBuffer::create(const void* data, size_t size) {
        // TODO: add config to select the API
        return new OGLVertexBuffer(data, size);
    }
}