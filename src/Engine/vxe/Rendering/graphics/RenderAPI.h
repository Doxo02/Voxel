#ifndef RENDERAPI_H
#define RENDERAPI_H

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace vxe
{
    class RenderAPI {
        public:
            virtual void init() = 0;
            virtual void setViewport(int x, int y, int width, int height) = 0;
            virtual void clear() = 0;
            virtual void drawVertexArray(const vxe::VertexArray* va, unsigned int count) = 0;
            virtual void drawElements(const vxe::VertexArray* va, unsigned int count) = 0;

            virtual void setClearColor(const glm::vec4& color) = 0;

            static RenderAPI* create();
    };
} // namespace vxe

#endif