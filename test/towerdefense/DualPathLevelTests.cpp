#include <catch2/catch_test_macros.hpp>
#include <Services/CampaignCatalog.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/LevelsCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace
{
	std::filesystem::path dataDir()
	{
		return (std::filesystem::path(__FILE__).parent_path() / ".." / ".."
			/ "app" / "TowerDefense" / "data")
			.lexically_normal();
	}

	std::string dataFile(const std::string& relative)
	{
		return (dataDir() / relative).string();
	}
}

TEST_CASE("level-4 has two named paths and a split wave", "[tower][paths][level-4]")
{
	tower::DamageTypeCatalog types;
	types.load(dataFile("damage-types.json"));
	tower::StatusCategoryCatalog categories;
	categories.load(dataFile("status-categories.json"));
	tower::StatusCatalog statuses;
	statuses.load(dataFile("statuses.json"), categories, types);
	tower::ProjectileCatalog projectiles;
	projectiles.load(dataFile("projectiles.json"), types, statuses);
	tower::WeaponCatalog weapons;
	weapons.load(dataFile("weapons.json"), projectiles);
	tower::CreepCatalog creeps;
	creeps.load(dataFile("creeps.json"), types, weapons);
	tower::EntityCatalog entities;
	entities.load(dataFile("entities.json"), types);
	tower::LevelCatalog level;
	level.load(dataFile("levels/level-4.json"), creeps, entities);

	REQUIRE(level.id() == "level-4");
	REQUIRE(level.paths().size() >= 2);
	std::unordered_set<std::string> pathNames;
	for (const auto& path : level.paths())
	{
		pathNames.insert(path.name);
	}
	REQUIRE(pathNames.contains("west"));
	REQUIRE(pathNames.contains("east"));
	CHECK(level.path("west").front().x == 0);
	CHECK(level.path("east").front().x == 7);

	bool splitWave = false;
	for (const auto& wave : level.waves())
	{
		std::unordered_set<std::string> streamPaths;
		for (const auto& stream : wave.streams)
		{
			streamPaths.insert(stream.pathName);
		}

		if (streamPaths.size() >= 2)
		{
			splitWave = true;
			break;
		}
	}

	CHECK(splitWave);
}

TEST_CASE("level-4 is listed in skirmish and not in campaign", "[tower][paths][level-4]")
{
	tower::LevelsCatalog levels;
	levels.load(dataFile("levels.json"));
	const auto* entry = levels.find("level-4");
	REQUIRE(entry != nullptr);
	CHECK(entry->file == "levels/level-4.json");

	tower::CampaignCatalog campaign;
	campaign.load(dataFile("campaign.json"));
	CHECK(campaign.find("level-4") == nullptr);
	for (const auto& node : campaign.nodes())
	{
		CHECK(node.level.find("level-4") == std::string::npos);
	}
}
