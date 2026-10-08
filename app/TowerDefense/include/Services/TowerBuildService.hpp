#pragma once

#include <helsinki/Engine/Scene/Scene.hpp>
#include <string_view>

namespace tower
{
	class WaveService;
	class GameStateService;
	class TowerCatalog;
	class LevelCatalog;
	class TowerSelectionService;

	class TowerBuildService
	{
	public:
		TowerBuildService(
			hl::Scene& scene,
			WaveService& wave,
			GameStateService& gameState,
			TowerCatalog& towers,
			LevelCatalog& level);

		bool tileOccupied(int tx, int tz) const;
		bool tryPlace(std::string_view defId, int x, int z);
		bool trySell(TowerSelectionService& selection);

	private:
		hl::Scene& _scene;
		WaveService& _wave;
		GameStateService& _gameState;
		TowerCatalog& _towers;
		LevelCatalog& _level;
	};
}
