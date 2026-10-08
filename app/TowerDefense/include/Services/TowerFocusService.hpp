#pragma once

#include <helsinki/Engine/Scene/Scene.hpp>

namespace tower
{
	class TowerSelectionService;
	class GameStateService;
	class TowerCatalog;
	class LevelCatalog;

	class TowerFocusService
	{
	public:
		TowerFocusService(
			hl::Scene& scene,
			TowerSelectionService& selection,
			GameStateService& gameState,
			TowerCatalog& towers,
			LevelCatalog& level);

		bool tryAssign(int targetEntityId);

	private:
		hl::Scene& _scene;
		TowerSelectionService& _selection;
		GameStateService& _gameState;
		TowerCatalog& _towers;
		LevelCatalog& _level;
	};
}
