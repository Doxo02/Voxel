#ifndef ELEMENT_BUFFER_H
#define ELEMENT_BUFFER_H

#include <GL/glew.h>

namespace gla {
    
    class ElementBuffer {
    public:
        ElementBuffer(void* data, unsigned int size);
        ~ElementBuffer();

        void bind();
        void unbind();

    private:
        unsigned int m_id;
    };
}

#endif // ELEMENT_BUFFER_OBJECT_H