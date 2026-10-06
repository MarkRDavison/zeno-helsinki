#include <Services/LevelCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <cmath>
#include <unordered_set>

namespace tower
{
	uint64_t LevelCatalog::tileKey(int x, int z)
	{
		return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32)
			| static_cast<uint32_t>(z);
	}

	void LevelCatalog::load(
		const std::string& path,
		const CreepCatalog& creeps,
		const EntityCatalog& entities)
	{
		_path.clear();
		_pathTiles.clear();
		_entities.clear();
		_waves.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		catalogJson::requireObjectRoot(doc, "level.json");
		const auto& root = *doc.m_Root;

		_id = catalogJson::requireString(root, "id", "level.json");
		_boardSize = catalogJson::requireIntAtLeast(root, "boardSize", 2, "level.json");
		_startGold = catalogJson::requireIntAtLeast(root, "startGold", 0, "level.json");
		_startLives = catalogJson::requireIntAtLeast(root, "startLives", 1, "level.json");
		_killGold = catalogJson::requireIntAtLeast(root, "killGold", 0, "level.json");
		_waveClearBonus = catalogJson::requireIntAtLeast(root, "waveClearBonus", 0, "level.json");
		_buildTimer = catalogJson::requirePositive(root, "buildTimer", "level.json");
		_spawnInterval = catalogJson::requirePositive(root, "spawnInterval", "level.json");

		const auto& pathNode = catalogJson::field(root, "path");
		if (pathNode.type != hl::JsonNode::Type::Array || pathNode.children.size() < 2)
		{
			catalogJson::fail("level.json: 'path' must be an array of at least 2 tiles");
		}

		for (const auto* step : pathNode.children)
		{
			if (step == nullptr || step->type != hl::JsonNode::Type::Array || step->children.size() != 2)
			{
				catalogJson::fail("level.json: each path tile must be [x, z]");
			}

			const auto* xNode = step->children[0];
			const auto* zNode = step->children[1];
			if (xNode == nullptr || zNode == nullptr
				|| xNode->type != hl::JsonNode::Type::ValueInteger
				|| zNode->type != hl::JsonNode::Type::ValueInteger)
			{
				catalogJson::fail("level.json: path tiles must be integer [x, z]");
			}

			TileCoord coord{ xNode->integer, zNode->integer };
			if (coord.x < 0 || coord.x >= _boardSize || coord.z < 0 || coord.z >= _boardSize)
			{
				catalogJson::fail("level.json: path tile is off the board");
			}

			if (!_path.empty())
			{
				const auto& prev = _path.back();
				const int dist = std::abs(coord.x - prev.x) + std::abs(coord.z - prev.z);
				if (dist != 1)
				{
					catalogJson::fail("level.json: path steps must be adjacent");
				}
			}

			_pathTiles.insert(tileKey(coord.x, coord.z));
			_path.push_back(coord);
		}

		const auto& entitiesNode = catalogJson::field(root, "entities");
		if (entitiesNode.type != hl::JsonNode::Type::Array)
		{
			catalogJson::fail("level.json: 'entities' must be an array");
		}

		std::unordered_set<uint64_t> occupied;
		for (const auto* row : entitiesNode.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("level.json: each entity placement must be an object");
			}

			LevelEntity placement;
			placement.id = catalogJson::requireString(*row, "id", "level.json");
			const auto& xNode = catalogJson::field(*row, "x");
			const auto& zNode = catalogJson::field(*row, "z");
			if (xNode.type != hl::JsonNode::Type::ValueInteger
				|| zNode.type != hl::JsonNode::Type::ValueInteger)
			{
				catalogJson::fail("level.json: entity x/z must be integers");
			}

			placement.x = xNode.integer;
			placement.z = zNode.integer;

			const auto* def = entities.find(placement.id);
			if (def == nullptr)
			{
				catalogJson::fail("level.json: unknown entity id '" + placement.id + "'");
			}

			placement.sizeX = def->sizeX;
			placement.sizeZ = def->sizeZ;
			for (int dz = 0; dz < placement.sizeZ; ++dz)
			{
				for (int dx = 0; dx < placement.sizeX; ++dx)
				{
					const int tx = placement.x + dx;
					const int tz = placement.z + dz;
					if (tx < 0 || tx >= _boardSize || tz < 0 || tz >= _boardSize)
					{
						catalogJson::fail("level.json: entity '" + placement.id + "' is off the board");
					}

					if (isPathTile(tx, tz))
					{
						catalogJson::fail("level.json: entity '" + placement.id + "' sits on the path");
					}

					const auto key = tileKey(tx, tz);
					if (!occupied.insert(key).second)
					{
						catalogJson::fail("level.json: entity '" + placement.id + "' overlaps another entity");
					}
				}
			}

			_entities.push_back(std::move(placement));
		}

		const auto& wavesNode = catalogJson::field(root, "waves");
		if (wavesNode.type != hl::JsonNode::Type::Array || wavesNode.children.empty())
		{
			catalogJson::fail("level.json: 'waves' must be a non-empty array");
		}

		std::unordered_set<std::string> waveIds;
		for (const auto* row : wavesNode.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail("level.json: each wave must be an object");
			}

			LevelWave wave;
			wave.id = catalogJson::requireString(*row, "id", "level.json");
			if (!waveIds.insert(wave.id).second)
			{
				catalogJson::fail("level.json: duplicate wave id '" + wave.id + "'");
			}

			const auto& spawnsNode = catalogJson::field(*row, "spawns");
			if (spawnsNode.type != hl::JsonNode::Type::Array || spawnsNode.children.empty())
			{
				catalogJson::fail("level.json: wave '" + wave.id + "' needs a non-empty spawns array");
			}

			for (const auto* spawnRow : spawnsNode.children)
			{
				if (spawnRow == nullptr || spawnRow->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail("level.json: each spawn must be an object");
				}

				WaveSpawn spawn;
				spawn.id = catalogJson::requireString(*spawnRow, "id", "level.json");
				spawn.count = catalogJson::requireIntAtLeast(*spawnRow, "count", 1, "level.json");
				if (creeps.find(spawn.id) == nullptr)
				{
					catalogJson::fail("level.json: unknown creep id '" + spawn.id + "'");
				}

				wave.spawns.push_back(std::move(spawn));
			}

			_waves.push_back(std::move(wave));
		}
	}

	const std::string& LevelCatalog::id() const
	{
		return _id;
	}

	int LevelCatalog::boardSize() const
	{
		return _boardSize;
	}

	int LevelCatalog::startGold() const
	{
		return _startGold;
	}

	int LevelCatalog::startLives() const
	{
		return _startLives;
	}

	int LevelCatalog::killGold() const
	{
		return _killGold;
	}

	int LevelCatalog::waveClearBonus() const
	{
		return _waveClearBonus;
	}

	float LevelCatalog::buildTimer() const
	{
		return _buildTimer;
	}

	float LevelCatalog::spawnInterval() const
	{
		return _spawnInterval;
	}

	int LevelCatalog::waveCount() const
	{
		return static_cast<int>(_waves.size());
	}

	const std::vector<TileCoord>& LevelCatalog::path() const
	{
		return _path;
	}

	const std::vector<LevelEntity>& LevelCatalog::entities() const
	{
		return _entities;
	}

	const std::vector<LevelWave>& LevelCatalog::waves() const
	{
		return _waves;
	}

	bool LevelCatalog::isOnBoard(int tx, int tz) const
	{
		return tx >= 0 && tx < _boardSize && tz >= 0 && tz < _boardSize;
	}

	bool LevelCatalog::isPathTile(int tx, int tz) const
	{
		return _pathTiles.contains(tileKey(tx, tz));
	}

	glm::vec3 LevelCatalog::tileCenter(int tx, int tz, float y) const
	{
		const float origin = (_boardSize * TileSize) * 0.5f - TileSize * 0.5f;
		return glm::vec3(
			static_cast<float>(tx) * TileSize - origin,
			y,
			static_cast<float>(tz) * TileSize - origin);
	}

	std::optional<TileCoord> LevelCatalog::worldToTile(const glm::vec3& hit) const
	{
		const float half = _boardSize * TileSize * 0.5f;
		const int tx = static_cast<int>(std::floor(hit.x + half));
		const int tz = static_cast<int>(std::floor(hit.z + half));
		if (!isOnBoard(tx, tz))
		{
			return std::nullopt;
		}

		return TileCoord{ tx, tz };
	}
}
