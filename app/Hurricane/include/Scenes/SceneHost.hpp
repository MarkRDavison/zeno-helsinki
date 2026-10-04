#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <optional>

namespace hur
{
	class SceneHost
	{
	public:
		SceneHost(hl::Engine& engine, hl::ServiceProvider& root);

		void goTitle();
		void goGame();
		void goSettings();
		void onSceneDestroyed();

	private:
		hl::Engine& _engine;
		hl::ServiceProvider& _root;
		std::optional<hl::ServiceProvider> _scope;
		std::optional<hl::ServiceProvider> _pendingScope;
	};
}
