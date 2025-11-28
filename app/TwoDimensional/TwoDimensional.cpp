#include <TwoDimensionalConfig.hpp>
#include <TwoDimensionalEngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>

static void registerServices(hl::ServiceProvider& services)
{
	services.registerService<hl::EventBus, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::Engine, hl::Engine, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::EngineConfiguration, hl::EngineConfiguration>(hl::ServiceLifetime::Singleton);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	hl::Engine& engine = serviceProvider.get<hl::Engine>();

	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();
	engineConfig.applyConfig("/data/config.json", std::string(td::TwoDimensionalConfig::RootPath));

	engine.init(engineConfig);
	engine.setScene(new td::TwoDimensionalEngineScene(engine, engineConfig));
	engine.run();

	return 0;
}