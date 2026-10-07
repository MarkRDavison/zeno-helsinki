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

	int WaveService::pendingSpawns() const
	{
		int total = 0;
		for (const auto& stream : _streams)
		{
			total += stream.pending;
		}

		return total;
	}

	void WaveService::fillQueues()
	{
		_streams.clear();
		const auto& wave = _level.waves()[static_cast<std::size_t>(_activeIndex)];
		for (const auto& stream : wave.streams)
		{
			StreamQueue queue;
			queue.pathName = stream.pathName;
			queue.spawnInterval = stream.spawnInterval;
			queue.spawnTimer = 0.0f;
			for (const auto& spawn : stream.spawns)
			{
				const auto* def = _creeps.find(spawn.id);
				for (int i = 0; i < spawn.count; ++i)
				{
					queue.queue.push_back(def);
				}
			}

			queue.pending = static_cast<int>(queue.queue.size());
			_streams.push_back(std::move(queue));
		}
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
		fillQueues();
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

		for (auto& stream : _streams)
		{
			if (stream.pending <= 0)
			{
				continue;
			}

			stream.spawnTimer -= delta;
		}
	}

	bool WaveService::takeSpawn()
	{
		if (!_inCombat || _gameState.matchEnded())
		{
			return false;
		}

		for (auto& stream : _streams)
		{
			if (stream.pending <= 0 || stream.spawnTimer > 0.0f)
			{
				continue;
			}

			_nextCreep = stream.queue[static_cast<std::size_t>(stream.queueIndex++)];
			_nextPathName = stream.pathName;
			stream.pending -= 1;
			stream.spawnTimer = stream.spawnInterval;
			return true;
		}

		return false;
	}

	bool WaveService::tryClear(bool boardEmpty)
	{
		// TODO stuck wave / can't finish (play-test): unarmed pile on an immortal path entity.
		if (!_inCombat || _gameState.matchEnded() || pendingSpawns() > 0 || !boardEmpty)
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

	std::string WaveService::hudWaveLine() const
	{
		const int index = hudWaveIndex();
		const auto& wave = _level.waves()[static_cast<std::size_t>(index - 1)];
		return "Wave " + wave.name + " (#" + std::to_string(index) + ")";
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

	const std::string& WaveService::nextPathName() const
	{
		if (_nextCreep == nullptr)
		{
			throw std::runtime_error("WaveService::nextPathName called with no pending creep");
		}

		return _nextPathName;
	}
}
