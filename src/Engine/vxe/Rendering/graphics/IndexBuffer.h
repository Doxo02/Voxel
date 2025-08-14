#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <cstddef>
#include <memory>

namespace vxe {
    class IndexBuffer {
        public:
            virtual ~IndexBuffer() = default;

            virtual void bind() const = 0;
            virtual void unbind() const = 0;

            static std::unique_ptr<IndexBuffer> create(unsigned int* indices, size_t count);
    };
}

#endif