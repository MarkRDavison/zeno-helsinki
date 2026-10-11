#include "SolarConfig.hpp"
#include <Scenes/TitleEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>

static void registerServices(hl::ServiceProvider& services)
{
	services.registerService<hl::EventBus, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::InputManager, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::Engine, hl::Engine, hl::EventBus, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::EngineConfiguration, hl::EngineConfiguration>(hl::ServiceLifetime::Singleton);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	auto& engine = serviceProvider.get<hl::Engine>();
	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();

	engineConfig.applyConfig("/data/config.json", std::string(slr::SolarConfig::RootPath));

	engine.init(engineConfig);
	engine.setScene(new slr::TitleEngineScene(engine, engineConfig));

	engine.run();

	return EXIT_SUCCESS;
}
