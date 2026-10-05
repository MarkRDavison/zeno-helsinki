#include <Services/WaveService.hpp>
#include <Services/GameStateService.hpp>
#include <SceneCatalog.hpp>

namespace tower
{
	WaveService::WaveService(GameStateService& gameState) :
		_gameState(gameState)
	{
	}

	bool WaveService::tryStart()
	{
		if (_inCombat || _gameState.matchEnded() || _wavesCompleted >= WaveCount)
		{
			return false;
		}

		_inCombat = true;
		_pendingSpawns = WaveCreepCount;
		_spawnTimer = 0.0f;
		return true;
	}

	void WaveService::tick(float delta)
	{
		if (!_inCombat || _gameState.matchEnded() || _pendingSpawns <= 0)
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
}
