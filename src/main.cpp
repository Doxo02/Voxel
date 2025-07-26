#include <spdlog/spdlog.h>

#include "App.h"

#define WIDTH 800
#define HEIGHT 600

int main(int, char**){
    spdlog::set_level(spdlog::level::debug);

    App app(WIDTH, HEIGHT, "Voxel Engine");
    
    if (!app.init()) {
        return 1;
    }
    app.run();
    app.terminate();

    return 0;
}
