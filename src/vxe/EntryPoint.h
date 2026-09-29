#ifndef VXE_ENTRYPOINT_H
#define VXE_ENTRYPOINT_H

extern vxe::Application* vxe::createApplication();

int main(int argc, char** argv) {
    auto app = vxe::createApplication();
    app->run();
    delete app;
}

#endif