#include "Application.h"
#include "Events/EventManager.h"
#include "Events/ApplicationEvent.h"
#include <chrono>

namespace vxe {
    Application::Application() {
        // Subscribe to application events using lambda to match the correct signature
        VXE_SUBSCRIBE(ApplicationUpdateEvent, [this](ApplicationUpdateEvent& e) {
            return this->onEvent(e);
        });
        VXE_SUBSCRIBE(ApplicationRenderEvent, [this](ApplicationRenderEvent& e) {
            return this->onEvent(e);
        });
    }

    Application::~Application() {
        // EventManager cleanup is handled automatically
    }

    void Application::run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        
        while (true) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Process any queued events
            VXE_PROCESS_EVENTS();
            
            // Dispatch application update event
            VXE_DISPATCH(ApplicationUpdateEvent, deltaTime);
            onUpdate(deltaTime);
            
            // Dispatch application render event
            VXE_DISPATCH(ApplicationRenderEvent);
            onRender();
            
            // Dispatch tick event
            VXE_DISPATCH(ApplicationTickEvent);
        }
    }
}