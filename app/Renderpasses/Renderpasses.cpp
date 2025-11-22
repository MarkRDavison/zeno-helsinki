#include <string>
#include <RenderpassesApplication.hpp>
#include "RenderpassesConfig.hpp"

#define ROOT_PATH(x) (std::string(rp::RenderpassesConfig::RootPath) + std::string(x))

int main()
{
    hl::EventBus eventBus;
    rp::RenderpassesApplication app(eventBus);
    app.init(rp::RenderpassConfig::Load(ROOT_PATH("/data/config.json")));
    app.run();

    return EXIT_SUCCESS;
}