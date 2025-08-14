#include "LinuxWindow.h"

#include <spdlog/spdlog.h>

namespace vxe {
    static uint8_t windowCount = 0;

    static void glfwErrorCallback(int error, const char* description) {
        spdlog::error("GLFW Error ({}): {}", error, description);
    }

    LinuxWindow::LinuxWindow(const WindowConfig& config) {
        m_data.width = config.width;
        m_data.height = config.height;
        m_data.title = config.title;

        if (windowCount == 0) {
            if (!glfwInit()) {
                spdlog::error("Failed to initialize GLFW!");
                exit(EXIT_FAILURE);
            }
            glfwSetErrorCallback(glfwErrorCallback);
        }
        windowCount++;
    }

    LinuxWindow::~LinuxWindow() {
        glfwDestroyWindow(m_window);
        windowCount--;

        if (windowCount == 0) {
            glfwTerminate();
        }
    }

    void LinuxWindow::init() {
        glfwSetWindowUserPointer(m_window, &m_data);

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*) glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;

            VXE_DISPATCH(WindowResizeEvent, width, height);
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window){
            VXE_DISPATCH(WindowCloseEvent);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            switch (action) {
                case GLFW_PRESS: {
                    VXE_DISPATCH(KeyPressedEvent, key, 0);
                } break;
                case GLFW_RELEASE: {
                    VXE_DISPATCH(KeyReleasedEvent, key);
                } break;
                case GLFW_REPEAT: {
                    VXE_DISPATCH(KeyPressedEvent, key, true);
                } break;
            }
        });

        glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode) {
            VXE_DISPATCH(KeyTypedEvent, keycode);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
            switch (action) {
                case GLFW_PRESS: {
                    VXE_DISPATCH(MouseButtonPressedEvent, button);
                } break;
                case GLFW_RELEASE: {
                    VXE_DISPATCH(MouseButtonReleasedEvent, button);
                } break;
            }
        });

        glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int focused) {
            if (focused == GLFW_TRUE) {
                VXE_DISPATCH(WindowFocusEvent);
            } else {
                VXE_DISPATCH(WindowLostFocusEvent);
            }
        });

        glfwSetWindowPosCallback(m_window, [](GLFWwindow* window, int xpos, int ypos) {
            VXE_DISPATCH(WindowMovedEvent, xpos, ypos);
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            VXE_DISPATCH(MouseScrolledEvent, (float) xOffset, (float) yOffset);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
            VXE_DISPATCH(MouseMovedEvent, xPos, yPos);
        });
    }

    void LinuxWindow::onUpdate() {
        glfwPollEvents();
    }

    bool LinuxWindow::isKeyDown(Key key) const {
        return glfwGetKey(m_window, static_cast<int>(key)) == GLFW_PRESS;
    }

    void LinuxWindow::setVSync(bool enabled) {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);
        
        m_data.vSync = enabled;
    }

    bool LinuxWindow::isVSync() const {
        return m_data.vSync;
    }

    void LinuxWindow::swapBuffer() const {
        glfwSwapBuffers(m_window);
    }

    void LinuxWindow::setOpenGLContext() {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow((int) m_data.width, (int) m_data.height, m_data.title.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window!");
        }
        glfwMakeContextCurrent(m_window);
    }

    void LinuxWindow::setCursorEnabled(bool enabled) {
        if (enabled)
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}
