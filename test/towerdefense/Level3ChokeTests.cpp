#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <filesystem>
#include <string>

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

	struct MatchCatalogs
	{
		tower::DamageTypeCatalog types;
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;
		tower::ProjectileCatalog projectiles;
		tower::WeaponCatalog weapons;
		tower::CreepCatalog creeps;
		tower::EntityCatalog entities;

		MatchCatalogs()
		{
			types.load(dataFile("damage-types.json"));
			categories.load(dataFile("status-categories.json"));
			statuses.load(dataFile("statuses.json"), categories, types);
			projectiles.load(dataFile("projectiles.json"), types, statuses);
			weapons.load(dataFile("weapons.json"), projectiles);
			creeps.load(dataFile("creeps.json"), types, weapons);
			entities.load(dataFile("entities.json"), types);
		}
	};
}

TEST_CASE("bite is physical and teeth uses bite", "[tower][paths][level-3]")
{
	MatchCatalogs catalogs;
	const auto* bite = catalogs.projectiles.find("bite");
	REQUIRE(bite != nullptr);
	CHECK(bite->damageType == "physical");
	const auto* teeth = catalogs.weapons.find("teeth");
	REQUIRE(teeth != nullptr);
	CHECK(teeth->projectile == "bite");
	CHECK(catalogs.projectiles.find("bolt") != nullptr);
}

TEST_CASE("brute slots teeth and runner is unarmed", "[tower][paths][level-3]")
{
	MatchCatalogs catalogs;
	const auto* brute = catalogs.creeps.find("brute");
	REQUIRE(brute != nullptr);
	REQUIRE(brute->slots.size() == 1);
	CHECK(brute->slots[0].id == "teeth");
	CHECK(brute->range == Catch::Approx(0.0f));
	const auto* runner = catalogs.creeps.find("runner");
	REQUIRE(runner != nullptr);
	CHECK(runner->slots.empty());
}

TEST_CASE("scenery rocks stay immortal", "[tower][paths][level-3]")
{
	MatchCatalogs catalogs;
	REQUIRE(catalogs.entities.find("rocks")->health == 0.0f);
	REQUIRE(catalogs.entities.find("choke_rock")->health == Catch::Approx(16.0f));
}

TEST_CASE("level-3 choke rock sits on main at 4,3 with brute and runner streams", "[tower][paths][level-3]")
{
	MatchCatalogs catalogs;
	tower::LevelCatalog level;
	level.load(dataFile("levels/level-3.json"), catalogs.creeps, catalogs.entities);

	CHECK(level.isPathTile(4, 3));
	bool foundRock = false;
	for (const auto& placed : level.entities())
	{
		if (placed.id == "choke_rock" && placed.x == 4 && placed.z == 3)
		{
			foundRock = true;
			break;
		}
	}
	CHECK(foundRock);

	REQUIRE_FALSE(level.waves().empty());
	const auto& wave1 = level.waves().front();
	REQUIRE(wave1.streams.size() == 2);
	CHECK(wave1.streams[0].name == "brutes");
	CHECK(wave1.streams[0].pathName == "main");
	REQUIRE(wave1.streams[0].spawns.size() == 1);
	CHECK(wave1.streams[0].spawns[0].id == "brute");
	CHECK(wave1.streams[1].name == "runners");
	CHECK(wave1.streams[1].pathName == "main");
	REQUIRE(wave1.streams[1].spawns.size() == 1);
	CHECK(wave1.streams[1].spawns[0].id == "runner");
}
