#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

#include "graphics/RenderAPI.h"
#include "graphics/Shader.h"
#include "graphics/ShaderStorageBuffer.h"

#include "Renderable.h"

namespace vxe {
    class Renderer {
        public:
            Renderer() = default;
            ~Renderer() = default;
            
            void init();
            
            void beginFrame();
            void submit(Renderable* object);
            void endFrame();

            RenderAPI* getAPI();
        private:
            std::vector<Renderable*> m_renderQueue;
            RenderAPI* m_api;
    };
}


#endif