#include <Services/WaveService.hpp>
#include <Services/GameStateService.hpp>
#include <SceneCatalog.hpp>
#include <algorithm>
#include <cmath>

namespace tower
{
	WaveService::WaveService(GameStateService& gameState) :
		_gameState(gameState)
	{
	}

	void WaveService::fillQueue()
	{
		_queueCount = 0;
		_queueIndex = 0;
		const auto& wave = Waves[_activeIndex];
		for (int i = 0; i < wave.runners && _queueCount < MaxWaveCreeps; ++i)
		{
			_queue[_queueCount++] = &CreepRunner;
		}
		for (int i = 0; i < wave.tanks && _queueCount < MaxWaveCreeps; ++i)
		{
			_queue[_queueCount++] = &CreepTank;
		}
		_pendingSpawns = _queueCount;
	}

	bool WaveService::tryStart()
	{
		if (_inCombat || _gameState.matchEnded() || _wavesCompleted >= WaveCount)
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
			if (_wavesCompleted >= WaveCount)
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

		_nextCreep = _queue[_queueIndex++];
		_pendingSpawns -= 1;
		_spawnTimer = WaveSpawnInterval;
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
		_gameState.addGold(WaveClearBonus);
		if (_wavesCompleted >= WaveCount)
		{
			_gameState.setWon();
		}
		else
		{
			_buildTimer = BuildTimerSeconds;
		}

		return true;
	}

	bool WaveService::inCombat() const
	{
		return _inCombat;
	}

	int WaveService::hudWaveIndex() const
	{
		if (_wavesCompleted >= WaveCount)
		{
			return WaveCount;
		}

		return _wavesCompleted + 1;
	}

	int WaveService::buildSecondsRemaining() const
	{
		if (_inCombat || _gameState.matchEnded() || _wavesCompleted >= WaveCount)
		{
			return 0;
		}

		return static_cast<int>(std::ceil(std::max(_buildTimer, 0.0f)));
	}

	const CreepDef& WaveService::nextCreep() const
	{
		return _nextCreep != nullptr ? *_nextCreep : CreepRunner;
	}
}
