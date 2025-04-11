#ifndef SHADER_STORAGE_BUFFER_H
#define SHADER_STORAGE_BUFFER_H

#include <GL/glew.h>

namespace gla {
    class ShaderStorageBuffer {
    public:
        ShaderStorageBuffer(void* data, unsigned int size, unsigned int index);
        ~ShaderStorageBuffer();

        void bind();
        void bindBase();
        void unbind();
    
    private:
        unsigned int m_id;
        unsigned int m_index;
    };
};

#endif // SHADER_STORAGE_BUFFER_H