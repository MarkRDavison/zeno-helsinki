#include <catch2/catch_test_macros.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <stdexcept>
#include <string>

namespace
{
	constexpr auto kTypes = R"json(
[
  { "id": "physical", "name": "Physical", "description": "Bolts and kinetic hits." },
  { "id": "fire", "name": "Fire", "description": "Explosions and burn hits." }
]
)json";

	tower::DamageTypeCatalog loadedTypes()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}
}

TEST_CASE("damage types include physical and fire", "[tower][combat][catalog]")
{
	const auto catalog = loadedTypes();
	REQUIRE(catalog.find("physical") != nullptr);
	REQUIRE(catalog.find("fire") != nullptr);
	CHECK(catalog.find("physical")->name == "Physical");
	CHECK(catalog.find("fire")->name == "Fire");
	CHECK_FALSE(catalog.find("fire")->description.empty());
}

TEST_CASE("damage types reject missing fire", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog catalog;
	CHECK_THROWS_AS(
		catalog.loadFromText(
			R"json([ { "id": "physical", "name": "Physical", "description": "x" } ])json",
			"damage-types.json"),
		std::runtime_error);
}

TEST_CASE("damage types reject missing physical", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog catalog;
	CHECK_THROWS_AS(
		catalog.loadFromText(
			R"json([ { "id": "fire", "name": "Fire", "description": "x" } ])json",
			"damage-types.json"),
		std::runtime_error);
}

TEST_CASE("damage types allow extra unused ids", "[tower][combat][catalog]")
{
	tower::DamageTypeCatalog catalog;
	CHECK_NOTHROW(catalog.loadFromText(
		R"json([
  { "id": "physical", "name": "Physical", "description": "x" },
  { "id": "fire", "name": "Fire", "description": "y" },
  { "id": "ice", "name": "Ice", "description": "unused until referenced" }
])json",
		"damage-types.json"));
	CHECK(catalog.contains("ice"));
}
