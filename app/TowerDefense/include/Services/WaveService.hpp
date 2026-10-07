#pragma once

#include <Services/CreepCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <string>
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
		std::string hudWaveLine() const;
		int buildSecondsRemaining() const;
		const CreepDef& nextCreep() const;
		const std::string& nextPathName() const;

	private:
		struct StreamQueue
		{
			std::string pathName;
			float spawnInterval = 0.0f;
			float spawnTimer = 0.0f;
			int pending = 0;
			int queueIndex = 0;
			std::vector<const CreepDef*> queue;
		};

		void fillQueues();
		int pendingSpawns() const;
		int waveCount() const;

		GameStateService& _gameState;
		CreepCatalog& _creeps;
		LevelCatalog& _level;
		int _wavesCompleted = 0;
		int _activeIndex = 0;
		bool _inCombat = false;
		float _buildTimer = 0.0f;
		std::vector<StreamQueue> _streams;
		const CreepDef* _nextCreep = nullptr;
		std::string _nextPathName;
	};
}
