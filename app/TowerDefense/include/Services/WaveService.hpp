#pragma once

#include <SceneCatalog.hpp>

namespace tower
{
	class GameStateService;

	class WaveService
	{
	public:
		explicit WaveService(GameStateService& gameState);

		bool tryStart();
		void tick(float delta);
		bool takeSpawn();
		bool tryClear(bool boardEmpty);
		bool inCombat() const;
		int hudWaveIndex() const;
		int buildSecondsRemaining() const;
		const WaveDef& active() const;

	private:
		GameStateService& _gameState;
		int _wavesCompleted = 0;
		int _activeIndex = 0;
		bool _inCombat = false;
		int _pendingSpawns = 0;
		float _spawnTimer = 0.0f;
		float _buildTimer = BuildTimerSeconds;
	};
}
