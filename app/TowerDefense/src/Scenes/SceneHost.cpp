#include <Scenes/SceneHost.hpp>
#include <Scenes/TowerDefenseGameEngineScene.hpp>
#include <Scenes/TowerDefenseSettingsEngineScene.hpp>
#include <Scenes/TowerDefenseTitleEngineScene.hpp>
#include <Services/GameStateService.hpp>
#include <Services/WaveService.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/EntityCatalog.hpp>
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

	void SceneHost::goGame()
	{
		_pendingScope = _root.createScope();
		auto& state = _pendingScope->get<GameStateService>();
		auto& waves = _pendingScope->get<WaveService>();
		_engine.setScene(new TowerDefenseGameEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this,
			state,
			waves,
			_root.get<CreepCatalog>(),
			_root.get<TowerCatalog>(),
			_root.get<EntityCatalog>(),
			_root.get<hl::audio::Audio>()));
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
