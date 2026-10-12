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
	REQUIRE(mineOutputPerTick(world.mines[0], SteelGoodId) == 100);
	REQUIRE(mineMaxStockpile(world.mines[0], SteelGoodId) == MineMaxStockpile_Steel);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(rimward->name == "Rimward");
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == 0);
}

TEST_CASE("stepGalaxy extracts onto the mine stockpile then advances tick", "[slr][Extraction]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
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
	world.trucks.clear();
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
	world.mines.push_back(Mine{
		.systemId = 99,
		.outputsPerTick = {
			MineOutput{ .goodId = SteelGoodId, .amountPerTick = 100, .maxStockpile = MineMaxStockpile_Steel } } });
	stepGalaxy(world);

	REQUIRE(world.systems.empty());
	REQUIRE(world.tick == 1);
}

TEST_CASE("a mine can extract more than one good", "[slr][Extraction]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	constexpr long long oreId = 2;
	world.mines.push_back(Mine{
		.systemId = 1,
		.outputsPerTick = {
			MineOutput{ .goodId = SteelGoodId, .amountPerTick = 100, .maxStockpile = MineMaxStockpile_Steel },
			MineOutput{ .goodId = oreId, .amountPerTick = 40, .maxStockpile = MineMaxStockpile_Steel } } });
	stepGalaxy(world);
	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == 100);
	REQUIRE(getStockpileAmount(world.systems.front(), oreId) == 40);
}

TEST_CASE("extraction stops at the mine max stockpile", "[slr][Extraction]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks[0].goodAmount = MineMaxStockpile_Steel - 30;
	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == MineMaxStockpile_Steel);
	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*rimward, SteelGoodId) == MineMaxStockpile_Steel);
}

}
}
