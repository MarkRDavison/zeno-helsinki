#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace FactoryRampTests
{

	constexpr long long KestrelSystemId = 2;
	constexpr long long JunctionSystemId = 3;

	void setSteelPile(StarSystem& system, long long amount)
	{
		for (auto& stock : system.stocks)
		{
			if (stock.goodId == SteelGoodId)
			{
				stock.goodAmount = amount;
				return;
			}
		}

		system.stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = amount });
	}

	Factory* factoryAtSystem(WorldData& world, long long systemId)
	{
		for (auto& factory : world.factories)
		{
			if (factory.systemId == systemId)
			{
				return &factory;
			}
		}

		return nullptr;
	}

TEST_CASE("missile output has a product ramp spec; steel output does not", "[slr][FactoryRamp]")
{
	REQUIRE(productRampSpecForGood(MissileGoodId) != nullptr);
	REQUIRE(productRampSpecForGood(SteelGoodId) == nullptr);
	REQUIRE(productRampSpecForGood(IronOreGoodId) == nullptr);
}

TEST_CASE("starter Kestrel missile plant starts at 10% efficiency per thousand", "[slr][FactoryRamp]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.factories.size() == 2);
	const Factory* kestrelFactory = nullptr;
	for (const auto& factory : world.factories)
	{
		if (factory.systemId == KestrelSystemId)
		{
			kestrelFactory = &factory;
		}
	}

	REQUIRE(kestrelFactory != nullptr);
	REQUIRE(kestrelFactory->outputGoodId == MissileGoodId);
	REQUIRE(kestrelFactory->productEfficiencyPerThousand == FactoryStartEfficiencyPerThousand);
	REQUIRE(consumerDesiredPile(world, KestrelSystemId, SteelGoodId) == FactoryMissileSteelDesiredPile);
}

TEST_CASE("Kestrel missile plant trickles then climbs with steel supplied", "[slr][FactoryRamp]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* kestrel = world.systemById(KestrelSystemId);
	REQUIRE(kestrel != nullptr);
	setSteelPile(*kestrel, 1000);
	Factory* missilePlant = factoryAtSystem(world, KestrelSystemId);
	REQUIRE(missilePlant != nullptr);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*kestrel, MissileGoodId) == 1);
	REQUIRE(getStockpileAmount(*kestrel, SteelGoodId) == 1000 - 1);
	REQUIRE(missilePlant->productEfficiencyPerThousand == FactoryStartEfficiencyPerThousand);

	for (int tick = 0; tick < 200; ++tick)
	{
		setSteelPile(*kestrel, 1000);
		stepGalaxy(world);
	}

	REQUIRE(missilePlant->productEfficiencyPerThousand > FactoryStartEfficiencyPerThousand);
	REQUIRE(getStockpileAmount(*kestrel, MissileGoodId) > 50);
}

TEST_CASE("Junction mill ignores ramp fields and stays full rate", "[slr][FactoryRamp]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	Factory* mill = factoryAtSystem(world, JunctionSystemId);
	REQUIRE(mill != nullptr);
	mill->productEfficiencyPerThousand = FactoryStartEfficiencyPerThousand;
	StarSystem* junction = world.systemById(JunctionSystemId);
	REQUIRE(junction != nullptr);
	junction->stocks[0].goodAmount = 50;

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*junction, IronOreGoodId) == 0);
	REQUIRE(getStockpileAmount(*junction, SteelGoodId) == 50);
	REQUIRE(mill->productRampUnitsProduced == 0);
}

TEST_CASE("factoryEffectiveAmount scales rated throughput", "[slr][FactoryRamp]")
{
	REQUIRE(factoryEffectiveAmount(10, FactoryFullEfficiencyPerThousand) == 10);
	REQUIRE(factoryEffectiveAmount(10, FactoryStartEfficiencyPerThousand) == 1);
	REQUIRE(factoryEffectiveAmount(10, 0) == 0);
}

}
}
