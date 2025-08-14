#ifndef VXE_APPLICATION_H
#define VXE_APPLICATION_H

namespace vxe {
    class Application {
        public:
            Application();
            virtual ~Application();

            void run();
    };

    // defined by client
    Application* createApplication();
}

#endif