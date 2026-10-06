#include "TowerDefenseConfig.hpp"
#include "Scenes/SceneHost.hpp"
#include <AudioCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/WaveService.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <helsinki/Audio/Audio.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>

static void registerServices(hl::ServiceProvider& services)
{
	services.registerService<hl::EventBus, hl::EventBus>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::InputManager, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::Engine, hl::Engine, hl::EventBus, hl::InputManager>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::EngineConfiguration, hl::EngineConfiguration>(hl::ServiceLifetime::Singleton);
	services.registerService<hl::audio::Audio, hl::audio::Audio>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::CreepCatalog, tower::CreepCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::TowerCatalog, tower::TowerCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::EntityCatalog, tower::EntityCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::GameStateService, tower::GameStateService>(hl::ServiceLifetime::Scoped);
	services.registerService<tower::WaveService, tower::WaveService, tower::GameStateService, tower::CreepCatalog>(hl::ServiceLifetime::Scoped);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	hl::Engine& engine = serviceProvider.get<hl::Engine>();

	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();
	engineConfig.applyConfig("/data/config.json", std::string(tower::TowerDefenseConfig::RootPath));
	serviceProvider.get<tower::CreepCatalog>().load(
		engineConfig.RootPath + "/data/creeps.json");
	serviceProvider.get<tower::TowerCatalog>().load(
		engineConfig.RootPath + "/data/towers.json");
	serviceProvider.get<tower::EntityCatalog>().load(
		engineConfig.RootPath + "/data/entities.json");

	engine.init(engineConfig);

	auto& audio = serviceProvider.get<hl::audio::Audio>();
	audio.init();
	tower::loadTowerAudio(audio, engineConfig.RootPath);

	tower::SceneHost sceneHost(engine, serviceProvider);
	sceneHost.goTitle();
	engine.run();

	audio.shutdown();

	return EXIT_SUCCESS;
}
