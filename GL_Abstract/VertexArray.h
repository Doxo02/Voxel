#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <GL/glew.h>

namespace gla {
    
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void bind();
        void unbind();

        void setAttribute(unsigned int index, unsigned int size, unsigned int type, bool normalized, unsigned int stride, void* pointer);

    private:
        unsigned int m_id;
    };
}

#endif // VERTEX_ARRAY_OBJECT_H