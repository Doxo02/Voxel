#include "IndexBuffer.h"
#include "RenderAPI.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <memory>

#include "../../Platform/OpenGL/ogl_IndexBuffer.h"
#include "../../Platform/OpenGL/ogl_RenderAPI.h"
#include "../../Platform/OpenGL/ogl_Shader.h"
#include "../../Platform/OpenGL/ogl_ShaderStorageBuffer.h"
#include "../../Platform/OpenGL/ogl_VertexArray.h"
#include "../../Platform/OpenGL/ogl_VertexBuffer.h"

namespace vxe {
    std::unique_ptr<IndexBuffer> IndexBuffer::create(unsigned int* indices, size_t count) {
        // TODO: add config to select the API
        return std::make_unique<OGLIndexBuffer>(indices, count);
    }

    std::unique_ptr<RenderAPI> RenderAPI::create() {
        // TODO: add config to select the API
        return std::make_unique<OGLRenderAPI>();
    }

    std::unique_ptr<Shader> Shader::create() {
        // TODO: add config to select the API
        return std::make_unique<OGLShader>();
    }

    std::unique_ptr<ShaderStorageBuffer> ShaderStorageBuffer::create(unsigned int index) {
        // TODO: add config to select the API
        return std::make_unique<OGLShaderStorageBuffer>(index);
    }

    std::unique_ptr<VertexArray> VertexArray::create() {
        // TODO: add config to select the API
        return std::make_unique<OGLVertexArray>();
    }

    std::unique_ptr<VertexBuffer> VertexBuffer::create(const void* data, size_t size) {
        // TODO: add config to select the API
        return std::make_unique<OGLVertexBuffer>(data, size);
    }
}