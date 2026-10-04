#include "HurricaneConfig.hpp"
#include "Scenes/SceneHost.hpp"
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <Services/GameStateService.hpp>
#include <Services/ResourceService.hpp>

static void registerServices(hl::ServiceProvider& services)
{
	services.registerService<hl::EventBus, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::InputManager, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::Engine, hl::Engine, hl::EventBus, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::EngineConfiguration, hl::EngineConfiguration>(hl::ServiceLifetime::Singleton);
	services.registerService<hur::GameStateService, hur::GameStateService>(hl::ServiceLifetime::Scoped);
	services.registerService<hur::ResourceService, hur::ResourceService>(hl::ServiceLifetime::Scoped);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	hl::Engine& engine = serviceProvider.get<hl::Engine>();

	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();
	engineConfig.applyConfig("/data/config.json", std::string(hur::HurricaneConfig::RootPath));

	engine.init(engineConfig);

	hur::SceneHost sceneHost(engine, serviceProvider);
	sceneHost.goTitle();
	engine.run();

	return EXIT_SUCCESS;
}
