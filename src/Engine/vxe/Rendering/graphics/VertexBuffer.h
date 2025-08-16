#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <cstddef>
#include <memory>

namespace vxe {
    enum class VertexAttribType {
        FLOAT, DOUBLE,
        BYTE, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT, UNSIGNED_INT,
    };

    struct VertexAttribLayout {
        VertexAttribType type;
        size_t index;
        size_t size;
    };

    class VertexBuffer {
        public:
            virtual ~VertexBuffer() = default;

            virtual void bind() const = 0;
            virtual void unbind() const = 0;
            virtual void setLayout(VertexAttribLayout* layouts, size_t size) const = 0;

            static std::unique_ptr<VertexBuffer> create(const void* data, size_t size);
    };
}

#endif