#ifndef OPENGL_RENDERAPI_H
#define OPENGL_RENDERAPI_H

#include "../RenderAPI.h"

#include <glm/vec4.hpp>

namespace vxe {
    class OGLRenderAPI : public RenderAPI {
        public:
            void init() override;
            void setViewport(int x, int y, int width, int height) override;
            void clear() override;
            void drawVertexArray(const VertexArray* va, unsigned int count) override;
            void drawElements(const vxe::VertexArray* va, unsigned int count) override;

            void setClearColor(const glm::vec4& color) override;
    };
}

#endif