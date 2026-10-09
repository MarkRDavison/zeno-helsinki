#include "DrillerConfig.hpp"
#include <Core/Session.hpp>
#include <Scenes/DrillerSplashEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <string>

static void registerServices(hl::ServiceProvider& services)
{
	services.registerService<hl::EventBus, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::InputManager, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::Engine, hl::Engine, hl::EventBus, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::EngineConfiguration, hl::EngineConfiguration>(hl::ServiceLifetime::Singleton);
	services.registerService<drl::Session, drl::Session>(hl::ServiceLifetime::Singleton);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	auto& engine = serviceProvider.get<hl::Engine>();
	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();
	auto& session = serviceProvider.get<drl::Session>();

	engineConfig.applyConfig("/data/config.json", std::string(drl::DrillerConfig::RootPath));
	session.loadGameSettings(engineConfig.RootPath + "/data/game.json");

	engine.init(engineConfig);
	engine.setScene(new drl::DrillerSplashEngineScene(engine, engineConfig, session));

	engine.run();

	return EXIT_SUCCESS;
}
