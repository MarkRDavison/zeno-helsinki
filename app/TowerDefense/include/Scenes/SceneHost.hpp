#pragma once

#include <helsinki/Engine/Engine.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <optional>
#include <string>

namespace tower
{
	class SceneHost
	{
	public:
		SceneHost(hl::Engine& engine, hl::ServiceProvider& root);

		void goTitle();
		void goLevelSelect();
		void goCampaignHub();
		void goCampaign(const std::string& nodeId);
		void goGame(const std::string& id);
		void goSettings();
		void onSceneDestroyed();

	private:
		void launchLoadedGame();

		hl::Engine& _engine;
		hl::ServiceProvider& _root;
		std::optional<hl::ServiceProvider> _scope;
		std::optional<hl::ServiceProvider> _pendingScope;
	};
}
