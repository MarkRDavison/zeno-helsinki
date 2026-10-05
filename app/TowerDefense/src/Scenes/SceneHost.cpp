#include <Scenes/SceneHost.hpp>
#include <Scenes/TowerDefenseGameEngineScene.hpp>
#include <Scenes/TowerDefenseTitleEngineScene.hpp>
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
		_engine.setScene(new TowerDefenseTitleEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this));
	}

	void SceneHost::goGame()
	{
		_engine.setScene(new TowerDefenseGameEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this,
			_root.get<hl::audio::Audio>()));
	}

	void SceneHost::onSceneDestroyed()
	{
	}
}
