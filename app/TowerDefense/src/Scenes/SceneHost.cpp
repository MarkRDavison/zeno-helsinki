#include <Scenes/SceneHost.hpp>
#include <Scenes/TowerDefenseCampaignHubEngineScene.hpp>
#include <Scenes/TowerDefenseGameEngineScene.hpp>
#include <Scenes/TowerDefenseLevelSelectEngineScene.hpp>
#include <Scenes/TowerDefenseSettingsEngineScene.hpp>
#include <Scenes/TowerDefenseTitleEngineScene.hpp>
#include <Services/GameStateService.hpp>
#include <Services/WaveService.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/LevelsCatalog.hpp>
#include <Services/CampaignCatalog.hpp>
#include <Services/CampaignEvaluator.hpp>
#include <Services/MatchContext.hpp>
#include <Services/ProfileService.hpp>
#include <Services/CatalogJson.hpp>
#include <helsinki/Engine/EngineConfiguration.hpp>
#include <helsinki/Audio/Audio.hpp>

namespace tower
{
	SceneHost::SceneHost(hl::Engine& engine, hl::ServiceProvider& root) :
		_engine(engine),
		_root(root)
	{
	}

	void SceneHost::goTitle()
	{
		_pendingScope.reset();
		_engine.setScene(new TowerDefenseTitleEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this));
	}

	void SceneHost::goLevelSelect()
	{
		_pendingScope.reset();
		_engine.setScene(new TowerDefenseLevelSelectEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this,
			_root.get<LevelsCatalog>()));
	}

	void SceneHost::goCampaignHub()
	{
		_pendingScope.reset();
		_engine.setScene(new TowerDefenseCampaignHubEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this,
			_root.get<CampaignCatalog>(),
			_root.get<ProfileService>().progress()));
	}

	void SceneHost::goCampaign(const std::string& nodeId)
	{
		const auto& catalog = _root.get<CampaignCatalog>();
		const auto* node = catalog.find(nodeId);
		if (node == nullptr)
		{
			catalogJson::fail("unknown campaign node '" + nodeId + "'");
		}

		const auto states = evaluateCampaign(catalog.data(), _root.get<ProfileService>().progress());
		const auto it = states.find(nodeId);
		if (it == states.end()
			|| (it->second != CampaignNodeState::Available
				&& it->second != CampaignNodeState::Cleared))
		{
			catalogJson::fail("campaign node '" + nodeId + "' is not playable");
		}

		auto& config = _root.get<hl::EngineConfiguration>();
		_root.get<LevelCatalog>().load(
			config.RootPath + "/data/" + node->level,
			_root.get<CreepCatalog>(),
			_root.get<EntityCatalog>());
		launchLoadedGame(&nodeId);
	}

	void SceneHost::goGame(const std::string& id)
	{
		const auto* entry = _root.get<LevelsCatalog>().find(id);
		if (entry == nullptr)
		{
			catalogJson::fail("unknown level id '" + id + "'");
		}

		auto& config = _root.get<hl::EngineConfiguration>();
		_root.get<LevelCatalog>().load(
			config.RootPath + "/data/" + entry->file,
			_root.get<CreepCatalog>(),
			_root.get<EntityCatalog>());
		launchLoadedGame(nullptr);
	}

	void SceneHost::launchLoadedGame(const std::string* campaignNodeId)
	{
		auto& config = _root.get<hl::EngineConfiguration>();
		_pendingScope = _root.createScope();
		auto& match = _pendingScope->get<MatchContext>();
		if (campaignNodeId != nullptr)
		{
			match.campaign = true;
			match.nodeId = *campaignNodeId;
			match.ownedTowers = _root.get<ProfileService>().profile().ownedTowers;
		}

		auto& state = _pendingScope->get<GameStateService>();
		auto& waves = _pendingScope->get<WaveService>();
		_engine.setScene(new TowerDefenseGameEngineScene(
			_engine,
			config,
			*this,
			state,
			match,
			waves,
			_root.get<CreepCatalog>(),
			_root.get<TowerCatalog>(),
			_root.get<WeaponCatalog>(),
			_root.get<ProjectileCatalog>(),
			_root.get<EntityCatalog>(),
			_root.get<LevelCatalog>(),
			_root.get<hl::audio::Audio>()));
	}

	void SceneHost::onCampaignWon(const std::string& nodeId)
	{
		_root.get<ProfileService>().recordWin(
			nodeId,
			_root.get<CampaignCatalog>().data());
	}

	void SceneHost::goSettings()
	{
		_pendingScope.reset();
		_engine.setScene(new TowerDefenseSettingsEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this,
			_root.get<hl::audio::Audio>()));
	}

	void SceneHost::onSceneDestroyed()
	{
		_scope = std::move(_pendingScope);
		_pendingScope.reset();
	}
}
