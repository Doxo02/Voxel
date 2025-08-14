#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <memory>
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace vxe
{
    class VertexArray {
        public:
            virtual ~VertexArray() = default;

            virtual void bind() const = 0;
            virtual void unbind() const = 0;

            virtual void addVertexBuffer(VertexBuffer* vertexBuffer) = 0;
            virtual void setIndexBuffer(IndexBuffer* indexBuffer) = 0;

            static std::unique_ptr<VertexArray> create();
    };
} // namespace vxe


#endif