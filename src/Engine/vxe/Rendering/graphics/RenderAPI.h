#ifndef RENDERAPI_H
#define RENDERAPI_H

#include <glm/glm.hpp>
#include <memory>

#include "VertexArray.h"
#include "../../Core/Window.h"

namespace vxe
{
    class RenderAPI {
        public:
            virtual void init(Window* window) = 0;
            virtual void setViewport(int x, int y, int width, int height) = 0;
            virtual void clear() = 0;
            virtual void drawVertexArray(const VertexArray* va, unsigned int count) = 0;
            virtual void drawElements(const VertexArray* va, unsigned int count) = 0;

            virtual void setClearColor(const glm::vec4& color) = 0;

            virtual void swapBuffer(Window* window) = 0;

            static std::unique_ptr<RenderAPI> create();
    };
}

#endif