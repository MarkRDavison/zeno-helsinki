#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/MatchContext.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

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
		tower::TowerCatalog towers;

		MatchCatalogs()
		{
			types.load(dataFile("damage-types.json"));
			categories.load(dataFile("status-categories.json"));
			statuses.load(dataFile("statuses.json"), categories, types);
			projectiles.load(dataFile("projectiles.json"), types, statuses);
			weapons.load(dataFile("weapons.json"), projectiles);
			towers.load(dataFile("towers.json"), weapons);
		}

		std::vector<std::string> catalogOrder() const
		{
			std::vector<std::string> ids;
			ids.reserve(towers.all().size());
			for (const auto& def : towers.all())
			{
				ids.push_back(def.id);
			}
			return ids;
		}
	};

	bool containsId(const std::vector<std::string>& ids, std::string_view id)
	{
		return std::find(ids.begin(), ids.end(), id) != ids.end();
	}
}

TEST_CASE("production slow and poison bolts list status ids; bolt stays empty", "[tower][combat][catalog]")
{
	MatchCatalogs catalogs;
	REQUIRE(catalogs.projectiles.find("bolt") != nullptr);
	CHECK(catalogs.projectiles.find("bolt")->statuses.empty());
	REQUIRE(catalogs.projectiles.find("slow_bolt") != nullptr);
	CHECK(catalogs.projectiles.find("slow_bolt")->statuses == std::vector<std::string>{ "slow" });
	REQUIRE(catalogs.projectiles.find("poison_bolt") != nullptr);
	CHECK(catalogs.projectiles.find("poison_bolt")->statuses == std::vector<std::string>{ "poison" });
}

TEST_CASE("production slow and poison towers slot their weapons", "[tower][combat][catalog]")
{
	MatchCatalogs catalogs;
	const auto* slow = catalogs.towers.find("slow");
	REQUIRE(slow != nullptr);
	REQUIRE(slow->weapons.size() == 1);
	CHECK(slow->weapons[0].id == "slow_cannon");
	CHECK(catalogs.weapons.find("slow_cannon")->projectile == "slow_bolt");
	const auto* poison = catalogs.towers.find("poison");
	REQUIRE(poison != nullptr);
	REQUIRE(poison->weapons.size() == 1);
	CHECK(poison->weapons[0].id == "poison_cannon");
	CHECK(catalogs.weapons.find("poison_cannon")->projectile == "poison_bolt");
}

TEST_CASE("skirmish placeable includes slow and poison", "[tower][match][catalog]")
{
	MatchCatalogs catalogs;
	tower::MatchContext match;
	const auto placeable = tower::matchPlaceableTowerIds(catalogs.catalogOrder(), match);
	CHECK(containsId(placeable, "slow"));
	CHECK(containsId(placeable, "poison"));
	CHECK(tower::matchAllowsTower("slow", match));
	CHECK(tower::matchAllowsTower("poison", match));
}

TEST_CASE("campaign owned single does not place slow or poison", "[tower][match][catalog]")
{
	MatchCatalogs catalogs;
	tower::MatchContext match;
	match.campaign = true;
	match.ownedTowers = { "single" };
	const auto placeable = tower::matchPlaceableTowerIds(catalogs.catalogOrder(), match);
	CHECK(containsId(placeable, "single"));
	CHECK_FALSE(containsId(placeable, "slow"));
	CHECK_FALSE(containsId(placeable, "poison"));
	CHECK_FALSE(tower::matchAllowsTower("slow", match));
	CHECK_FALSE(tower::matchAllowsTower("poison", match));
}
