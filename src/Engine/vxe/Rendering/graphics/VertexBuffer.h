#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <cstddef>
#include <memory>

namespace vxe {
    class VertexBuffer {
        public:
            virtual ~VertexBuffer() = default;

            virtual void bind() const = 0;
            virtual void unbind() const = 0;

            static std::unique_ptr<VertexBuffer> create(const void* data, size_t size);
    };
}

#endif