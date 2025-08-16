#ifndef OPENGL_VERTEX_ARRAY_H
#define OPENGL_VERTEX_ARRAY_H

#include "../../Rendering/graphics/VertexArray.h"

#include <GL/glew.h>
#include <vector>

namespace vxe {
    class OGLVertexArray : public VertexArray {
        public:
            OGLVertexArray();
            ~OGLVertexArray();

            void bind() const override;
            void unbind() const override;

            void addVertexBuffer(VertexBuffer* vertexBuffer, VertexAttribLayout* layouts, size_t size) override;
            void setIndexBuffer(IndexBuffer* indexBuffer) override;
        
        private:
            GLuint m_id;

            std::vector<VertexBuffer*> m_vertexBuffers;
            IndexBuffer* m_indexBuffer;
    };
}


#endif