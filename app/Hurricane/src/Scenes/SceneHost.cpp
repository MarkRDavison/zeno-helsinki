#include <Scenes/SceneHost.hpp>
#include <Scenes/HurricaneGameEngineScene.hpp>
#include <Scenes/HurricaneSettingsEngineScene.hpp>
#include <Scenes/HurricaneTitleEngineScene.hpp>
#include <Services/GameStateService.hpp>
#include <Services/ResourceService.hpp>
#include <helsinki/Engine/EngineConfiguration.hpp>

namespace hur
{
	SceneHost::SceneHost(hl::Engine& engine, hl::ServiceProvider& root) :
		_engine(engine),
		_root(root)
	{
	}

	void SceneHost::goTitle()
	{
		_pendingScope.reset();
		_engine.setScene(new HurricaneTitleEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this));
	}

	void SceneHost::goGame()
	{
		_pendingScope = _root.createScope();
		auto& state = _pendingScope->get<GameStateService>();
		auto& resources = _pendingScope->get<ResourceService>();
		_engine.setScene(new HurricaneGameEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			state,
			resources,
			*this));
	}

	void SceneHost::goSettings()
	{
		_pendingScope.reset();
		_engine.setScene(new HurricaneSettingsEngineScene(
			_engine,
			_root.get<hl::EngineConfiguration>(),
			*this));
	}

	void SceneHost::onSceneDestroyed()
	{
		_scope = std::move(_pendingScope);
		_pendingScope.reset();
	}
}
