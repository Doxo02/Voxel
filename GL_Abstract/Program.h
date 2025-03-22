#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>
#include <string>

namespace gla {
    
    class Program {
    public:
        Program(const std::string& vertexShader, const std::string& fragmentShader);
        ~Program();

        void bind();
        void unbind();

    private:
        unsigned int m_id;

        unsigned int compileShader(unsigned int type, const std::string& source);
    };
}

#endif // PROGRAM_H