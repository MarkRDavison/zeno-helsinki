#include "TowerDefenseConfig.hpp"
#include "Scenes/SceneHost.hpp"
#include <AudioCatalog.hpp>
#include <Services/GameStateService.hpp>
#include <Services/WaveService.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/LevelsCatalog.hpp>
#include <Services/CampaignCatalog.hpp>
#include <Services/ResearchCatalog.hpp>
#include <Services/ProfileService.hpp>
#include <Services/MatchContext.hpp>
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
	services.registerService<tower::DamageTypeCatalog, tower::DamageTypeCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::CreepCatalog, tower::CreepCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::ProjectileCatalog, tower::ProjectileCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::StatusCategoryCatalog, tower::StatusCategoryCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::StatusCatalog, tower::StatusCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::WeaponCatalog, tower::WeaponCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::TowerCatalog, tower::TowerCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::EntityCatalog, tower::EntityCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::LevelCatalog, tower::LevelCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::LevelsCatalog, tower::LevelsCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::CampaignCatalog, tower::CampaignCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::ResearchCatalog, tower::ResearchCatalog>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::ProfileService, tower::ProfileService>(hl::ServiceLifetime::Singleton);
	services.registerService<tower::MatchContext, tower::MatchContext>(hl::ServiceLifetime::Scoped);
	services.registerService<tower::GameStateService, tower::GameStateService, tower::LevelCatalog, tower::MatchContext>(hl::ServiceLifetime::Scoped);
	services.registerService<tower::WaveService, tower::WaveService, tower::GameStateService, tower::CreepCatalog, tower::LevelCatalog>(hl::ServiceLifetime::Scoped);
}

int main()
{
	hl::ServiceProvider serviceProvider;

	registerServices(serviceProvider);

	hl::Engine& engine = serviceProvider.get<hl::Engine>();

	auto& engineConfig = serviceProvider.get<hl::EngineConfiguration>();
	engineConfig.applyConfig("/data/config.json", std::string(tower::TowerDefenseConfig::RootPath));
	serviceProvider.get<tower::DamageTypeCatalog>().load(
		engineConfig.RootPath + "/data/damage-types.json");
	serviceProvider.get<tower::StatusCategoryCatalog>().load(
		engineConfig.RootPath + "/data/status-categories.json");
	serviceProvider.get<tower::StatusCatalog>().load(
		engineConfig.RootPath + "/data/statuses.json",
		serviceProvider.get<tower::StatusCategoryCatalog>(),
		serviceProvider.get<tower::DamageTypeCatalog>());
	serviceProvider.get<tower::CreepCatalog>().load(
		engineConfig.RootPath + "/data/creeps.json",
		serviceProvider.get<tower::DamageTypeCatalog>());
	serviceProvider.get<tower::ProjectileCatalog>().load(
		engineConfig.RootPath + "/data/projectiles.json",
		serviceProvider.get<tower::DamageTypeCatalog>(),
		serviceProvider.get<tower::StatusCatalog>());
	serviceProvider.get<tower::WeaponCatalog>().load(
		engineConfig.RootPath + "/data/weapons.json",
		serviceProvider.get<tower::ProjectileCatalog>());
	serviceProvider.get<tower::TowerCatalog>().load(
		engineConfig.RootPath + "/data/towers.json",
		serviceProvider.get<tower::WeaponCatalog>());
	serviceProvider.get<tower::EntityCatalog>().load(
		engineConfig.RootPath + "/data/entities.json");
	serviceProvider.get<tower::LevelsCatalog>().load(
		engineConfig.RootPath + "/data/levels.json");
	serviceProvider.get<tower::CampaignCatalog>().load(
		engineConfig.RootPath + "/data/campaign.json");
	serviceProvider.get<tower::ResearchCatalog>().load(
		engineConfig.RootPath + "/data/research.json");
	if (const auto savePath = tower::campaignSavePath(); savePath.has_value())
	{
		serviceProvider.get<tower::ProfileService>().load(*savePath, true);
	}
	else
	{
		serviceProvider.get<tower::ProfileService>().load({}, true);
	}
	serviceProvider.get<tower::ProfileService>().syncFromResearch(
		serviceProvider.get<tower::ResearchCatalog>().data());

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
