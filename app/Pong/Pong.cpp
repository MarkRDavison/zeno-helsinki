#include "PongConfig.hpp"
#include <PongEngineScene.hpp>
#include <PongTitleEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>

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

	engineConfig.applyConfig("/data/config.json", std::string(pong::PongConfig::RootPath));

	engine.init(engineConfig);
	//engine.setScene(new pong::PongEngineScene(engine, engineConfig));
	engine.setScene(new pong::PongTitleEngineScene(engine, engineConfig));
	
	engine.run();

	return EXIT_SUCCESS;
}