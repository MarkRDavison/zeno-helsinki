#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace YardTests
{

TEST_CASE("Helion yard wants a steel buffer; Rimward mine does not", "[slr][Yard]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.yards.size() == 1);
	REQUIRE(world.yards.front().systemId == 1);
	REQUIRE(quantityForGood(world.yards.front().desiredPiles, SteelGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 1, SteelGoodId) == 1000);
	REQUIRE(consumerDesiredPile(world, 5, SteelGoodId) == 0);

	const StarSystem* helion = world.systemById(1);
	const StarSystem* rimward = world.systemById(5);
	REQUIRE(helion != nullptr);
	REQUIRE(rimward != nullptr);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1000);
	REQUIRE(getNeed(*rimward, SteelGoodId) == 0);
}

TEST_CASE("mine systems do not get desired pile from digging", "[slr][Yard]")
{
	WorldData world = makeStarterMap();
	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks[0].goodAmount = 500;
	setNeedFromConsumerDesiredPile(world, *rimward, SteelGoodId);
	REQUIRE(getNeed(*rimward, SteelGoodId) == 0);
	REQUIRE(consumerDesiredPile(world, 5, SteelGoodId) == 0);
}

TEST_CASE("need on a consumer system comes from the yard desired pile", "[slr][Yard]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{ .goodId = SteelGoodId });
	world.yards.push_back(Yard{
		.systemId = 1,
		.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = 1000 } } });
	setNeedFromConsumerDesiredPile(world, world.systems.front(), SteelGoodId);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 1000);
}

TEST_CASE("a yard can want more than one good", "[slr][Yard]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	constexpr long long platesId = 2;
	world.yards.push_back(Yard{
		.systemId = 1,
		.desiredPiles = {
			GoodQuantity{ .goodId = SteelGoodId, .amount = 1000 },
			GoodQuantity{ .goodId = platesId, .amount = 200 } } });
	setNeedsFromConsumerDesiredPiles(world, world.systems.front());
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 1000);
	REQUIRE(getNeed(world.systems.front(), platesId) == 200);
}

}
}
