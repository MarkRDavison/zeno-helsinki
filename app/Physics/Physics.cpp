#include "PhysicsConfig.hpp"
#include <Scenes/TitleEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Physics/Physics.hpp>
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
	engineConfig.applyConfig("/data/config.json", std::string(phys::PhysicsConfig::RootPath));

	engine.init(engineConfig);

	hl::physics::Context physicsContext;
	if (!physicsContext.init())
	{
		return EXIT_FAILURE;
	}

	engine.setScene(new phys::TitleEngineScene(engine, engineConfig));
	engine.run();
	physicsContext.shutdown();
	return EXIT_SUCCESS;
}
