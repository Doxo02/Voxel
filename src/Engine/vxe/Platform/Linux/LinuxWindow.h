#ifndef VXE_GLFW_WINDOW_H
#define VXE_GLFW_WINDOW_H

#include "../../Core/Window.h"

#include <GLFW/glfw3.h>

namespace vxe {
    class LinuxWindow : public Window {
        public:
            LinuxWindow(const WindowConfig& config);
            ~LinuxWindow() override;

            void init() override;

            void onUpdate() override;

            unsigned int getWidth() const override { return m_data.width; }
            unsigned int getHeight() const override { return m_data.height; }
            bool isKeyDown(Key key) const override;

            void setVSync(bool enabled) override;
            bool isVSync() const override;

            virtual void* getNativeWindow() const { return m_window; }

            void swapBuffer() const override;

            void setOpenGLContext() override;

            void setCursorEnabled(bool enabled) override;

        private:
            GLFWwindow* m_window;

            struct WindowData {
                std::string title;
                unsigned int width, height;
                bool vSync;
            };

            WindowData m_data;

            bool openGLContext = false;
    };
}

#endif