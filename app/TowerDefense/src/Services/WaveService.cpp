#include <Services/WaveService.hpp>
#include <Services/GameStateService.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tower
{
	WaveService::WaveService(
		GameStateService& gameState,
		CreepCatalog& creeps,
		LevelCatalog& level) :
		_gameState(gameState),
		_creeps(creeps),
		_level(level),
		_buildTimer(level.buildTimer())
	{
	}

	int WaveService::waveCount() const
	{
		return _level.waveCount();
	}

	void WaveService::fillQueue()
	{
		_queue.clear();
		_queueIndex = 0;
		const auto& wave = _level.waves()[static_cast<std::size_t>(_activeIndex)];
		for (const auto& spawn : wave.spawns)
		{
			const auto* def = _creeps.find(spawn.id);
			for (int i = 0; i < spawn.count; ++i)
			{
				_queue.push_back(def);
			}
		}

		_pendingSpawns = static_cast<int>(_queue.size());
	}

	bool WaveService::tryStart()
	{
		if (_inCombat || _gameState.matchEnded() || _wavesCompleted >= waveCount())
		{
			return false;
		}

		_inCombat = true;
		_buildTimer = 0.0f;
		_activeIndex = _wavesCompleted;
		_spawnTimer = 0.0f;
		fillQueue();
		return true;
	}

	void WaveService::tick(float delta)
	{
		if (_gameState.matchEnded())
		{
			return;
		}

		if (!_inCombat)
		{
			if (_wavesCompleted >= waveCount())
			{
				return;
			}

			_buildTimer -= delta;
			if (_buildTimer <= 0.0f)
			{
				tryStart();
			}

			return;
		}

		if (_pendingSpawns <= 0)
		{
			return;
		}

		_spawnTimer -= delta;
	}

	bool WaveService::takeSpawn()
	{
		if (!_inCombat || _gameState.matchEnded() || _pendingSpawns <= 0 || _spawnTimer > 0.0f)
		{
			return false;
		}

		_nextCreep = _queue[static_cast<std::size_t>(_queueIndex++)];
		_pendingSpawns -= 1;
		_spawnTimer = _level.spawnInterval();
		return true;
	}

	bool WaveService::tryClear(bool boardEmpty)
	{
		if (!_inCombat || _gameState.matchEnded() || _pendingSpawns > 0 || !boardEmpty)
		{
			return false;
		}

		_inCombat = false;
		_wavesCompleted += 1;
		_gameState.addGold(_level.waveClearBonus());
		if (_wavesCompleted >= waveCount())
		{
			_gameState.setWon();
		}
		else
		{
			_buildTimer = _level.buildTimer();
		}

		return true;
	}

	bool WaveService::inCombat() const
	{
		return _inCombat;
	}

	int WaveService::hudWaveIndex() const
	{
		if (_wavesCompleted >= waveCount())
		{
			return waveCount();
		}

		return _wavesCompleted + 1;
	}

	int WaveService::buildSecondsRemaining() const
	{
		if (_inCombat || _gameState.matchEnded() || _wavesCompleted >= waveCount())
		{
			return 0;
		}

		return static_cast<int>(std::ceil(std::max(_buildTimer, 0.0f)));
	}

	const CreepDef& WaveService::nextCreep() const
	{
		if (_nextCreep == nullptr)
		{
			throw std::runtime_error("WaveService::nextCreep called with no pending creep");
		}

		return *_nextCreep;
	}
}
