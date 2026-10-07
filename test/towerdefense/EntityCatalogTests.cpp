#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Combat.hpp>
#include <Services/DamageTypeCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
	constexpr auto kTypes = R"json(
[
  { "id": "physical", "name": "Physical", "description": "Bolts." },
  { "id": "fire", "name": "Fire", "description": "Fire." },
  { "id": "poison", "name": "Poison", "description": "Poison." }
]
)json";

	tower::DamageTypeCatalog types()
	{
		tower::DamageTypeCatalog catalog;
		catalog.loadFromText(kTypes, "damage-types.json");
		return catalog;
	}
}

TEST_CASE("omitted entity health is immortal", "[tower][entity][catalog]")
{
	const auto loaded = types();
	tower::EntityCatalog catalog;
	catalog.loadFromText(
		R"json([
  { "id": "tree", "model": "detail_tree", "size": [1, 1] },
  { "id": "rocks", "model": "detail_rocks", "size": [1, 1], "health": 0 }
])json",
		"entities.json",
		loaded);
	REQUIRE(catalog.find("tree")->health == 0.0f);
	REQUIRE(catalog.find("rocks")->health == 0.0f);
}

TEST_CASE("entity health above 0 dies at current <= 0", "[tower][entity][combat]")
{
	float current = 10.0f;
	tower::applyHit(current, 4.0f, 0.0f);
	CHECK(current == Catch::Approx(6.0f));
	CHECK_FALSE(tower::isDead(current));
	tower::applyHit(current, 10.0f, 0.0f);
	CHECK(tower::isDead(current));
}

TEST_CASE("entity missing resist key is 0", "[tower][entity][catalog]")
{
	const auto loaded = types();
	tower::EntityCatalog catalog;
	catalog.loadFromText(
		R"json([ { "id": "rocks", "model": "detail_rocks", "size": [1, 1], "health": 8 } ])json",
		"entities.json",
		loaded);
	const auto* rocks = catalog.find("rocks");
	REQUIRE(rocks != nullptr);
	CHECK(rocks->health == Catch::Approx(8.0f));
	CHECK(tower::resistOf(rocks->resist, "physical") == 0.0f);
}

TEST_CASE("entity resist above 1 loads clamped and warns", "[tower][entity][catalog]")
{
	const auto loaded = types();
	tower::EntityCatalog catalog;
	std::ostringstream captured;
	auto* previous = std::clog.rdbuf(captured.rdbuf());
	catalog.loadFromText(
		R"json([
  { "id": "rocks", "model": "detail_rocks", "size": [1, 1], "resist": { "physical": 1.5 } }
])json",
		"entities.json",
		loaded);
	std::clog.rdbuf(previous);
	CHECK(catalog.find("rocks")->resist.at("physical") == Catch::Approx(1.0f));
	CHECK(captured.str().find("clamped to 1.0") != std::string::npos);
}
