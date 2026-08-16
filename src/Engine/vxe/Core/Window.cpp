#include "Window.h"

#include <memory>

#ifdef _WIN32
    #include "../Platform/Windows/WindowsWindow.h"
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    #include "../Platform/Linux/LinuxWindow.h"
#endif

namespace vxe {
    std::unique_ptr<Window> Window::Create(const WindowConfig& config) {
        #ifdef _WIN32
            return std::make_unique<WindowsWindow>(config);
        #elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
            return std::make_unique<LinuxWindow>(config);
        #endif
    }
}