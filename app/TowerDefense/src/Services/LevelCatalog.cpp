#include <Services/LevelCatalog.hpp>
#include <Services/CatalogJson.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <cmath>
#include <stdexcept>
#include <unordered_set>

namespace tower
{
	uint64_t LevelCatalog::tileKey(int x, int z)
	{
		return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32)
			| static_cast<uint32_t>(z);
	}

	namespace
	{
		std::vector<TileCoord> parsePathTiles(
			const hl::JsonNode& pathNode,
			int boardWidth,
			int boardDepth,
			const char* file,
			const std::string& pathName)
		{
			if (pathNode.type != hl::JsonNode::Type::Array || pathNode.children.size() < 2)
			{
				catalogJson::fail(
					std::string(file) + ": path '" + pathName + "' must be an array of at least 2 tiles");
			}

			std::vector<TileCoord> tiles;
			for (const auto* step : pathNode.children)
			{
				if (step == nullptr || step->type != hl::JsonNode::Type::Array || step->children.size() != 2)
				{
					catalogJson::fail(std::string(file) + ": each path tile must be [x, z]");
				}

				const auto* xNode = step->children[0];
				const auto* zNode = step->children[1];
				if (xNode == nullptr || zNode == nullptr
					|| xNode->type != hl::JsonNode::Type::ValueInteger
					|| zNode->type != hl::JsonNode::Type::ValueInteger)
				{
					catalogJson::fail(std::string(file) + ": path tiles must be integer [x, z]");
				}

				TileCoord coord{ xNode->integer, zNode->integer };
				if (coord.x < 0 || coord.x >= boardWidth || coord.z < 0 || coord.z >= boardDepth)
				{
					catalogJson::fail(std::string(file) + ": path tile is off the board");
				}

				if (!tiles.empty())
				{
					const auto& prev = tiles.back();
					const int dist = std::abs(coord.x - prev.x) + std::abs(coord.z - prev.z);
					if (dist != 1)
					{
						catalogJson::fail(std::string(file) + ": path steps must be adjacent");
					}
				}

				tiles.push_back(coord);
			}

			return tiles;
		}

		std::vector<WaveSpawn> parseSpawns(
			const hl::JsonNode& spawnsNode,
			const CreepCatalog& creeps,
			const char* file,
			const std::string& context)
		{
			if (spawnsNode.type != hl::JsonNode::Type::Array || spawnsNode.children.empty())
			{
				catalogJson::fail(std::string(file) + ": " + context + " needs a non-empty spawns array");
			}

			std::vector<WaveSpawn> spawns;
			for (const auto* spawnRow : spawnsNode.children)
			{
				if (spawnRow == nullptr || spawnRow->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail(std::string(file) + ": each spawn must be an object");
				}

				WaveSpawn spawn;
				spawn.id = catalogJson::requireString(*spawnRow, "id", file);
				spawn.count = catalogJson::requireIntAtLeast(*spawnRow, "count", 1, file);
				if (creeps.find(spawn.id) == nullptr)
				{
					catalogJson::fail(std::string(file) + ": unknown creep id '" + spawn.id + "'");
				}

				spawns.push_back(std::move(spawn));
			}

			return spawns;
		}
	}

	void LevelCatalog::load(
		const std::string& path,
		const CreepCatalog& creeps,
		const EntityCatalog& entities)
	{
		loadFromText(hl::String::readFile(path), "level.json", creeps, entities);
	}

	void LevelCatalog::loadFromText(
		const std::string& text,
		const char* file,
		const CreepCatalog& creeps,
		const EntityCatalog& entities)
	{
		_paths.clear();
		_pathByName.clear();
		_pathTiles.clear();
		_entities.clear();
		_waves.clear();

		const auto doc = hl::Json::parseFromText(text);
		catalogJson::requireObjectRoot(doc, file);
		const auto& root = *doc.m_Root;

		if (catalogJson::findChild(root, "path") != nullptr)
		{
			catalogJson::fail(std::string(file) + ": singular 'path' is invalid; use 'paths'");
		}

		if (catalogJson::findChild(root, "spawnInterval") != nullptr)
		{
			catalogJson::fail(std::string(file) + ": level-wide 'spawnInterval' is invalid; put it on each stream");
		}

		_id = catalogJson::requireString(root, "id", file);
		const auto board = catalogJson::requireBoardSize(root, file);
		_boardWidth = board.first;
		_boardDepth = board.second;
		_startGold = catalogJson::requireIntAtLeast(root, "startGold", 0, file);
		_startLives = catalogJson::requireIntAtLeast(root, "startLives", 1, file);
		_killGold = catalogJson::requireIntAtLeast(root, "killGold", 0, file);
		_waveClearBonus = catalogJson::requireIntAtLeast(root, "waveClearBonus", 0, file);
		_buildTimer = catalogJson::requirePositive(root, "buildTimer", file);

		const auto& pathsNode = catalogJson::field(root, "paths");
		if (pathsNode.type != hl::JsonNode::Type::Array || pathsNode.children.empty())
		{
			catalogJson::fail(std::string(file) + ": 'paths' must be a non-empty array");
		}

		for (const auto* row : pathsNode.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": each path entry must be an object");
			}

			NamedPath named;
			named.name = catalogJson::requireString(*row, "name", file);
			if (_pathByName.contains(named.name))
			{
				catalogJson::fail(std::string(file) + ": duplicate path name '" + named.name + "'");
			}

			named.path = parsePathTiles(
				catalogJson::field(*row, "path"),
				_boardWidth,
				_boardDepth,
				file,
				named.name);
			for (const auto& tile : named.path)
			{
				_pathTiles.insert(tileKey(tile.x, tile.z));
			}

			_pathByName.emplace(named.name, _paths.size());
			_paths.push_back(std::move(named));
		}

		const auto& entitiesNode = catalogJson::field(root, "entities");
		if (entitiesNode.type != hl::JsonNode::Type::Array)
		{
			catalogJson::fail(std::string(file) + ": 'entities' must be an array");
		}

		std::unordered_set<uint64_t> occupied;
		for (const auto* row : entitiesNode.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": each entity placement must be an object");
			}

			LevelEntity placement;
			placement.id = catalogJson::requireString(*row, "id", file);
			const auto& xNode = catalogJson::field(*row, "x");
			const auto& zNode = catalogJson::field(*row, "z");
			if (xNode.type != hl::JsonNode::Type::ValueInteger
				|| zNode.type != hl::JsonNode::Type::ValueInteger)
			{
				catalogJson::fail(std::string(file) + ": entity x/z must be integers");
			}

			placement.x = xNode.integer;
			placement.z = zNode.integer;

			const auto* def = entities.find(placement.id);
			if (def == nullptr)
			{
				catalogJson::fail(std::string(file) + ": unknown entity id '" + placement.id + "'");
			}

			placement.sizeX = def->sizeX;
			placement.sizeZ = def->sizeZ;
			for (int dz = 0; dz < placement.sizeZ; ++dz)
			{
				for (int dx = 0; dx < placement.sizeX; ++dx)
				{
					const int tx = placement.x + dx;
					const int tz = placement.z + dz;
					if (tx < 0 || tx >= _boardWidth || tz < 0 || tz >= _boardDepth)
					{
						catalogJson::fail(std::string(file) + ": entity '" + placement.id + "' is off the board");
					}

					const auto key = tileKey(tx, tz);
					if (!occupied.insert(key).second)
					{
						catalogJson::fail(
							std::string(file) + ": entity '" + placement.id + "' overlaps another entity");
					}
				}
			}

			_entities.push_back(std::move(placement));
		}

		const auto& wavesNode = catalogJson::field(root, "waves");
		if (wavesNode.type != hl::JsonNode::Type::Array || wavesNode.children.empty())
		{
			catalogJson::fail(std::string(file) + ": 'waves' must be a non-empty array");
		}

		std::unordered_set<std::string> waveNames;
		for (const auto* row : wavesNode.children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				catalogJson::fail(std::string(file) + ": each wave must be an object");
			}

			if (catalogJson::findChild(*row, "spawns") != nullptr
				&& catalogJson::findChild(*row, "streams") == nullptr)
			{
				catalogJson::fail(std::string(file) + ": old wave shape (bare 'spawns') is invalid");
			}

			LevelWave wave;
			wave.name = catalogJson::requireString(*row, "name", file);
			if (!waveNames.insert(wave.name).second)
			{
				catalogJson::fail(std::string(file) + ": duplicate wave name '" + wave.name + "'");
			}

			const auto& streamsNode = catalogJson::field(*row, "streams");
			if (streamsNode.type != hl::JsonNode::Type::Array || streamsNode.children.empty())
			{
				catalogJson::fail(std::string(file) + ": wave '" + wave.name + "' needs a non-empty streams array");
			}

			std::unordered_set<std::string> streamNames;
			for (const auto* streamRow : streamsNode.children)
			{
				if (streamRow == nullptr || streamRow->type != hl::JsonNode::Type::Object)
				{
					catalogJson::fail(std::string(file) + ": each stream must be an object");
				}

				LevelStream stream;
				stream.name = catalogJson::requireString(*streamRow, "name", file);
				if (!streamNames.insert(stream.name).second)
				{
					catalogJson::fail(
						std::string(file) + ": duplicate stream name '" + stream.name
						+ "' in wave '" + wave.name + "'");
				}

				stream.pathName = catalogJson::requireString(*streamRow, "path", file);
				if (!_pathByName.contains(stream.pathName))
				{
					catalogJson::fail(std::string(file) + ": unknown stream path '" + stream.pathName + "'");
				}

				stream.spawnInterval = catalogJson::requirePositive(*streamRow, "spawnInterval", file);
				stream.spawns = parseSpawns(
					catalogJson::field(*streamRow, "spawns"),
					creeps,
					file,
					"stream '" + stream.name + "'");
				wave.streams.push_back(std::move(stream));
			}

			_waves.push_back(std::move(wave));
		}
	}

	const std::string& LevelCatalog::id() const
	{
		return _id;
	}

	int LevelCatalog::boardWidth() const
	{
		return _boardWidth;
	}

	int LevelCatalog::boardDepth() const
	{
		return _boardDepth;
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

	int LevelCatalog::waveCount() const
	{
		return static_cast<int>(_waves.size());
	}

	const std::vector<NamedPath>& LevelCatalog::paths() const
	{
		return _paths;
	}

	const std::vector<TileCoord>& LevelCatalog::path(std::string_view name) const
	{
		const auto it = _pathByName.find(std::string(name));
		if (it == _pathByName.end())
		{
			throw std::runtime_error("unknown path '" + std::string(name) + "'");
		}

		return _paths[it->second].path;
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
		return tx >= 0 && tx < _boardWidth && tz >= 0 && tz < _boardDepth;
	}

	bool LevelCatalog::isPathTile(int tx, int tz) const
	{
		return _pathTiles.contains(tileKey(tx, tz));
	}

	glm::vec3 LevelCatalog::tileCenter(int tx, int tz, float y) const
	{
		const float originX = (_boardWidth * TileSize) * 0.5f - TileSize * 0.5f;
		const float originZ = (_boardDepth * TileSize) * 0.5f - TileSize * 0.5f;
		return glm::vec3(
			static_cast<float>(tx) * TileSize - originX,
			y,
			static_cast<float>(tz) * TileSize - originZ);
	}

	std::optional<TileCoord> LevelCatalog::worldToTile(const glm::vec3& hit) const
	{
		const float halfW = _boardWidth * TileSize * 0.5f;
		const float halfD = _boardDepth * TileSize * 0.5f;
		const int tx = static_cast<int>(std::floor(hit.x + halfW));
		const int tz = static_cast<int>(std::floor(hit.z + halfD));
		if (!isOnBoard(tx, tz))
		{
			return std::nullopt;
		}

		return TileCoord{ tx, tz };
	}
}
