#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/DamageTypeCatalog.hpp>
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

	struct ProductionCatalogs
	{
		tower::DamageTypeCatalog types;
		tower::StatusCategoryCatalog categories;
		tower::StatusCatalog statuses;
		tower::ProjectileCatalog projectiles;
		tower::WeaponCatalog weapons;
		tower::CreepCatalog creeps;

		ProductionCatalogs()
		{
			types.load(dataFile("damage-types.json"));
			categories.load(dataFile("status-categories.json"));
			statuses.load(dataFile("statuses.json"), categories, types);
			projectiles.load(dataFile("projectiles.json"), types, statuses);
			weapons.load(dataFile("weapons.json"), projectiles);
			creeps.load(dataFile("creeps.json"), types, weapons);
		}
	};
}

TEST_CASE("production creeps.json has runner and tank", "[tower][catalog][smoke]")
{
	ProductionCatalogs catalogs;
	REQUIRE(catalogs.creeps.find("runner") != nullptr);
	REQUIRE(catalogs.creeps.find("tank") != nullptr);
}

TEST_CASE("production tank physical resist is 0.25", "[tower][catalog][smoke]")
{
	ProductionCatalogs catalogs;
	const auto* tank = catalogs.creeps.find("tank");
	REQUIRE(tank != nullptr);
	REQUIRE(tank->resist.contains("physical"));
	CHECK(tank->resist.at("physical") == Catch::Approx(0.25f));
}

TEST_CASE("production bolt statuses stay empty", "[tower][catalog][smoke]")
{
	ProductionCatalogs catalogs;
	const auto* bolt = catalogs.projectiles.find("bolt");
	REQUIRE(bolt != nullptr);
	CHECK(bolt->statuses.empty());
}
