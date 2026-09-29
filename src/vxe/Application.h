#ifndef VXE_APPLICATION_H
#define VXE_APPLICATION_H

#include "Events/Event.h"

namespace vxe {
    class Application {
        public:
            Application();
            virtual ~Application();

            virtual void run();
            
            // Virtual event handler that can be overridden by derived applications
            virtual bool onEvent(Event& event) { return false; }
            
            // Called every frame for updates
            virtual void onUpdate(float deltaTime) {}
            
            // Called every frame for rendering
            virtual void onRender() {}
    };

    // defined by client
    Application* createApplication();
}

#endif