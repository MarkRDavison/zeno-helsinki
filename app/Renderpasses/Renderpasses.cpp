#include <string>
#include <iostream>

#include <RenderpassesApplication.hpp>
#include "RenderpassesConfig.hpp"

#define ROOT_PATH(x) (std::string(rp::RenderpassesConfig::RootPath) + std::string(x))

int main()
{
    hl::EventBus eventBus;
    auto config = rp::RenderpassConfig::Load(ROOT_PATH("/data/config.json"));
    rp::RenderpassesApplication app(eventBus);


    app.init(config);
    app.run();

    return EXIT_SUCCESS;
}