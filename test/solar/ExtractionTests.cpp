#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace ExtractionTests
{

TEST_CASE("starter map has one Rimward steel mine with empty stockpile", "[slr][Extraction]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.mines.size() == 1);
	REQUIRE(world.mines[0].systemId == 5);
	REQUIRE(world.mines[0].goodId == SteelGoodId);
	REQUIRE(world.mines[0].outputPerTick == 100);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(rimward->name == "Rimward");
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 0);
}

TEST_CASE("stepGalaxy extracts onto the mine stockpile then advances tick", "[slr][Extraction]")
{
	WorldData world = makeStarterMap();
	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	const StarSystem* helion = world.systemById(1);
	REQUIRE(rimward != nullptr);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 100);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(world.tick == 1);
}

TEST_CASE("two galaxy steps double stockpile", "[slr][Extraction]")
{
	WorldData world = makeStarterMap();
	stepGalaxy(world);
	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 200);
	REQUIRE(world.tick == 2);
}

TEST_CASE("mine on unknown system does not invent a system", "[slr][Extraction]")
{
	WorldData world;
	world.mines.push_back(Mine{ .systemId = 99, .goodId = SteelGoodId, .outputPerTick = 100 });
	stepGalaxy(world);

	REQUIRE(world.systems.empty());
	REQUIRE(world.tick == 1);
}

}
}
