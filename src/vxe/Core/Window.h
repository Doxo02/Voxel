#ifndef VXE_WINDOW_H
#define VXE_WINDOW_H

#include "../Events/Events.h"

#include <string>
#include <stdint.h>
#include <memory>

#include "KeyCodes.h"

namespace vxe {
    struct WindowConfig {
        std::string title;
        uint32_t width, height;

        WindowConfig(const std::string& title = "Voxel Engine", uint32_t width = 800, uint32_t height = 600) 
            : title(title), width(width), height(height) {}
    };

    class Window {
        public:
            virtual ~Window() = default;

            virtual void init() = 0;

            virtual void onUpdate() = 0;

            virtual uint32_t getWidth() const = 0;
            virtual uint32_t getHeight() const = 0;
            virtual bool isKeyDown(Key key) const = 0;

            virtual void setVSync(bool enabled) = 0;
            virtual bool isVSync() const = 0;

            virtual void* getNativeWindow() const = 0;

            virtual void swapBuffer() const = 0;
            virtual void setOpenGLContext() = 0;

            virtual void setCursorEnabled(bool enabled) = 0;

            static std::unique_ptr<Window> Create(const WindowConfig& config = WindowConfig());
    };
}

#endif