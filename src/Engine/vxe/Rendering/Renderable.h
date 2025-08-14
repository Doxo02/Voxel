#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <memory>

#include "graphics/RenderAPI.h"

namespace vxe {
    class Renderable {
        public:
            virtual ~Renderable() = default;
            virtual void draw(RenderAPI* api) = 0;
    };
}

#endif