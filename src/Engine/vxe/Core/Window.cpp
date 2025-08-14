#include "Window.h"

#include <memory>
#include "../Platform/Linux/LinuxWindow.h"

namespace vxe {
    std::unique_ptr<Window> Window::Create(const WindowConfig& config) {
        return std::make_unique<LinuxWindow>(config);
    }
}