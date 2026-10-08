#pragma once

#include <helsinki/Engine/Scene/Scene.hpp>
#include <string_view>

namespace hl
{
	class ResourceManager;

	namespace audio
	{
		class Audio;
	}
}

namespace tower
{
	class WaveService;
	class GameStateService;
	class TowerCatalog;
	class LevelCatalog;
	class TowerSelectionService;
	struct MatchContext;

	class TowerBuildService
	{
	public:
		TowerBuildService(
			hl::Scene& scene,
			WaveService& wave,
			GameStateService& gameState,
			TowerCatalog& towers,
			LevelCatalog& level,
			MatchContext& match,
			hl::audio::Audio* audio = nullptr);

		void setResourceManager(hl::ResourceManager* resources);
		bool tileOccupied(int tx, int tz) const;
		bool tryPlace(std::string_view defId, int x, int z);
		bool trySell(TowerSelectionService& selection);

	private:
		hl::Scene& _scene;
		WaveService& _wave;
		GameStateService& _gameState;
		TowerCatalog& _towers;
		LevelCatalog& _level;
		MatchContext& _match;
		hl::audio::Audio* _audio = nullptr;
		hl::ResourceManager* _resources = nullptr;
	};
}
