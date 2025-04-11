#ifndef VERTX_BUFFER_H
#define VERTX_BUFFER_H

#include <GL/glew.h>

namespace gla {
    
    class VertexBuffer {
    public:
        VertexBuffer(void* data, unsigned int size);
        ~VertexBuffer();

        void bind();
        void unbind();

    private:
        unsigned int m_id;
    };
}

#endif // VERTX_BUFFER_OBJECT_H