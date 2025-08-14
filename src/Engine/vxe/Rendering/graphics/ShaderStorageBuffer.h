#ifndef SHADER_STORAGE_BUFFER
#define SHADER_STORAGE_BUFFER

#include <memory>

namespace vxe {
    class ShaderStorageBuffer {
        public:
            virtual ~ShaderStorageBuffer() = default;

            virtual void bind() const = 0;
            virtual void bindBase() const = 0;
            virtual void unbind() const = 0;

            static std::unique_ptr<ShaderStorageBuffer> create(void *data, unsigned int size, unsigned int index);
    };
}

#endif