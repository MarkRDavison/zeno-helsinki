#include <string>
#include <iostream>

#include "RenderpassesConfig.hpp"

#include <RenderpassesApplication.hpp>

#define ROOT_PATH(x) (std::string(ex::ExampleConfig::RootPath) + std::string(x))

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600; 

int main()
{
    hl::EventBus eventBus;
    rp::RenderpassesApplication app(eventBus);

    app.init(WIDTH, HEIGHT, "Vulkan Helsinki Renderpasses");
    app.run();

    return EXIT_SUCCESS;
}