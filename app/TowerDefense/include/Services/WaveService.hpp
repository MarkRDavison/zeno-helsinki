#pragma once

#include <Services/CreepCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <vector>

namespace tower
{
	class GameStateService;

	class WaveService
	{
	public:
		WaveService(GameStateService& gameState, CreepCatalog& creeps, LevelCatalog& level);

		bool tryStart();
		void tick(float delta);
		bool takeSpawn();
		bool tryClear(bool boardEmpty);
		bool inCombat() const;
		int hudWaveIndex() const;
		int buildSecondsRemaining() const;
		const CreepDef& nextCreep() const;

	private:
		void fillQueue();
		int waveCount() const;

		GameStateService& _gameState;
		CreepCatalog& _creeps;
		LevelCatalog& _level;
		int _wavesCompleted = 0;
		int _activeIndex = 0;
		bool _inCombat = false;
		int _pendingSpawns = 0;
		float _spawnTimer = 0.0f;
		float _buildTimer = 0.0f;
		std::vector<const CreepDef*> _queue;
		int _queueIndex = 0;
		const CreepDef* _nextCreep = nullptr;
	};
}
