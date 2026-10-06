#pragma once

#include <SceneCatalog.hpp>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>
#include <cstdint>

namespace tower
{
	class CreepCatalog;
	class EntityCatalog;

	struct LevelEntity
	{
		std::string id;
		int x = 0;
		int z = 0;
		int sizeX = 1;
		int sizeZ = 1;
	};

	struct WaveSpawn
	{
		std::string id;
		int count = 0;
	};

	struct LevelWave
	{
		std::string id;
		std::vector<WaveSpawn> spawns;
	};

	class LevelCatalog
	{
	public:
		void load(
			const std::string& path,
			const CreepCatalog& creeps,
			const EntityCatalog& entities);

		const std::string& id() const;
		int boardWidth() const;
		int boardDepth() const;
		int startGold() const;
		int startLives() const;
		int killGold() const;
		int waveClearBonus() const;
		float buildTimer() const;
		float spawnInterval() const;
		int waveCount() const;
		const std::vector<TileCoord>& path() const;
		const std::vector<LevelEntity>& entities() const;
		const std::vector<LevelWave>& waves() const;

		bool isOnBoard(int tx, int tz) const;
		bool isPathTile(int tx, int tz) const;
		glm::vec3 tileCenter(int tx, int tz, float y = 0.0f) const;
		std::optional<TileCoord> worldToTile(const glm::vec3& hit) const;

	private:
		static uint64_t tileKey(int x, int z);

		std::string _id;
		int _boardWidth = 0;
		int _boardDepth = 0;
		int _startGold = 0;
		int _startLives = 0;
		int _killGold = 0;
		int _waveClearBonus = 0;
		float _buildTimer = 0.0f;
		float _spawnInterval = 0.0f;
		std::vector<TileCoord> _path;
		std::unordered_set<uint64_t> _pathTiles;
		std::vector<LevelEntity> _entities;
		std::vector<LevelWave> _waves;
	};
}
