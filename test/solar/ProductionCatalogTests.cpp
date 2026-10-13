#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace ProductionCatalogTests
{

long long countAvonFactories(const WorldData& world)
{
	long long count = 0;
	for (const auto& factory : world.factories)
	{
		if (factory.systemId == AvonSystemId)
		{
			++count;
		}
	}

	return count;
}

long long countLanesTouchingSystem(const WorldData& world, long long systemId)
{
	long long count = 0;
	for (const auto& lane : world.hyperlanes)
	{
		if (lane.a == systemId || lane.b == systemId)
		{
			++count;
		}
	}

	return count;
}

Factory* assemblerAtAvon(WorldData& world)
{
	for (auto& factory : world.factories)
	{
		if (factory.systemId == AvonSystemId && factory.outputGoodId == PrototypeMissileMk1GoodId)
		{
			return &factory;
		}
	}

	return nullptr;
}

TEST_CASE("starter map adds Avon linked only to Kestrel", "[slr][ProductionCatalog]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systems.size() >= 9);
	const StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	REQUIRE(avon->name == "Avon");
	REQUIRE(countLanesTouchingSystem(world, AvonSystemId) == 1);
	REQUIRE(countAvonFactories(world) == 4);
}

TEST_CASE("starter Avon factories copy recipe inputs", "[slr][ProductionCatalog]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.factories[2].systemId == AvonSystemId);
	REQUIRE(world.factories[2].inputsPerTick.size() == 1);
	REQUIRE(world.factories[2].inputsPerTick[0].goodId == SteelGoodId);
	REQUIRE(world.factories[2].inputsPerTick[0].amount == FactoryAvonEngineSteelPerTick);
	REQUIRE(world.factories[5].outputGoodId == PrototypeMissileMk1GoodId);
	REQUIRE(world.factories[5].inputsPerTick.size() == 3);
	REQUIRE(world.factories[5].ticksPerOutput == FactoryAvonAssemblerTicksPerOutput);
}

TEST_CASE("catalog recipes match Avon proving-ground costs and caps", "[slr][ProductionCatalog]")
{
	const FactoryRecipe* engine = factoryRecipeById(FactoryRecipeId_MissileEngineMk1);
	const FactoryRecipe* warhead = factoryRecipeById(FactoryRecipeId_MissileWarheadMk1);
	const FactoryRecipe* seeker = factoryRecipeById(FactoryRecipeId_MissileSeekerMk1);
	const FactoryRecipe* assembled = factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1);
	REQUIRE(engine != nullptr);
	REQUIRE(warhead != nullptr);
	REQUIRE(seeker != nullptr);
	REQUIRE(assembled != nullptr);
	REQUIRE(engine->inputsPerTick.size() == 1);
	REQUIRE(engine->inputsPerTick[0].goodId == SteelGoodId);
	REQUIRE(engine->inputsPerTick[0].amount == FactoryAvonEngineSteelPerTick);
	REQUIRE(engine->outputMaxStockpile == AvonComponentMaxStockpile);
	REQUIRE(warhead->inputsPerTick[0].amount == FactoryAvonWarheadSteelPerTick);
	REQUIRE(seeker->inputsPerTick[0].amount == FactoryAvonSeekerSteelPerTick);
	REQUIRE(assembled->inputsPerTick.size() == 3);
	REQUIRE(assembled->inputsPerTick[0].amount == FactoryAvonAssemblerComponentInputPerTick);
	REQUIRE(assembled->outputAmountPerTick == FactoryAvonAssemblerOutputPerTick);
	REQUIRE(assembled->ticksPerOutput == FactoryAvonAssemblerTicksPerOutput);
	REQUIRE(assembled->outputGoodId == PrototypeMissileMk1GoodId);
	REQUIRE(assembled->outputMaxStockpile == AvonPrototypeMissileMaxStockpile);
}

TEST_CASE("steel recipe factories emit a component the same tick; assembler waits two", "[slr][ProductionCatalog]")
{
	using Catch::Approx;
	WorldData world;
	StarSystem avon{ .id = AvonSystemId, .name = "Avon" };
	avon.stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = AvonStarterSteelStockpile });
	avon.stocks.push_back(GoodStock{ .goodId = MissileEngineMk1GoodId, .goodAmount = 0 });
	avon.stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 0 });
	avon.stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 0 });
	avon.stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk1GoodId, .goodAmount = 0 });
	world.systems.push_back(avon);
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileEngineMk1)));
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileWarheadMk1)));
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileSeekerMk1)));
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1)));
	REQUIRE(world.factories[0].inputsPerTick.size() == 1);
	REQUIRE(world.factories[0].outputAmountPerTick == FactoryAvonComponentOutputPerTick);
	REQUIRE(world.factories[0].ticksPerOutput == 1);
	REQUIRE(world.factories[3].ticksPerOutput == FactoryAvonAssemblerTicksPerOutput);

	applyFactoryProduction(world);

	REQUIRE(getStockpileAmount(world.systems[0], SteelGoodId) < AvonStarterSteelStockpile);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId) == Approx(0.f));

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId) == Approx(1.f));
}

TEST_CASE(
	"starter map Avon assembler emits one prototype missile every two stepGalaxy ticks",
	"[slr][ProductionCatalog]")
{
	using Catch::Approx;
	WorldData world = makeStarterMap();
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	REQUIRE(getStockpileAmount(*avon, SteelGoodId) == Approx(AvonStarterSteelStockpile));

	const Factory* assembler = assemblerAtAvon(world);
	REQUIRE(assembler != nullptr);
	REQUIRE(assembler->outputAmountPerTick == FactoryAvonAssemblerOutputPerTick);
	REQUIRE(assembler->ticksPerOutput == FactoryAvonAssemblerTicksPerOutput);
	REQUIRE(getStockpileAmount(*avon, PrototypeMissileMk1GoodId) == Approx(0.f));

	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*avon, PrototypeMissileMk1GoodId) == Approx(0.f));

	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*avon, PrototypeMissileMk1GoodId) == Approx(1.f));
}

TEST_CASE("Avon produces no prototype missiles without steel", "[slr][ProductionCatalog]")
{
	using Catch::Approx;
	WorldData world = makeStarterMap();
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	for (auto& stock : avon->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 0.f;
			break;
		}
	}

	stepGalaxy(world);
	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*avon, PrototypeMissileMk1GoodId) == Approx(0.f));
}

TEST_CASE("assembler emits one prototype missile after two ticks", "[slr][ProductionCatalog]")
{
	using Catch::Approx;
	WorldData world;
	StarSystem avon{ .id = AvonSystemId, .name = "Avon" };
	avon.stocks.push_back(GoodStock{ .goodId = MissileEngineMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk1GoodId, .goodAmount = 0.f });
	world.systems.push_back(avon);
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1)));
	REQUIRE(world.factories[0].outputAmountPerTick == FactoryAvonAssemblerOutputPerTick);
	REQUIRE(world.factories[0].ticksPerOutput == FactoryAvonAssemblerTicksPerOutput);

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId) == Approx(0.f));
	REQUIRE(getStockpileAmount(world.systems[0], MissileEngineMk1GoodId) == Approx(0.f));
	REQUIRE(world.factories[0].remainingOutputTicks == 1);

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId) == Approx(1.f));
	REQUIRE(world.factories[0].remainingOutputTicks == 0);
}

TEST_CASE("Avon assembler stalls when a component is missing", "[slr][ProductionCatalog]")
{
	WorldData world;
	StarSystem system{ .id = AvonSystemId, .name = "Avon" };
	system.stocks.push_back(GoodStock{ .goodId = MissileEngineMk1GoodId, .goodAmount = 1.f });
	system.stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 1.f });
	system.stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 0.f });
	system.stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk1GoodId, .goodAmount = 0.f });
	world.systems.push_back(system);
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1)));

	const long long before = getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId);
	stepGalaxy(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk1GoodId) == before);
}

TEST_CASE("prototype missile good has no empire consumers in starter", "[slr][ProductionCatalog]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(consumerDesiredPile(world, AvonSystemId, PrototypeMissileMk1GoodId) == 0);
	REQUIRE(consumerDesiredPile(world, 2, PrototypeMissileMk1GoodId) == 0);
}

}
}
