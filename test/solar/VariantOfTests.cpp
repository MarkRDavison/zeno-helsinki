#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace VariantOfTests
{

TEST_CASE("retarget Mk1 assembler to Mk2 keeps most efficiency", "[slr][VariantOf]")
{
	WorldData world = makeStarterMap();
	Factory* assembler = nullptr;
	for (auto& factory : world.factories)
	{
		if (factory.systemId == AvonSystemId && factory.outputGoodId == PrototypeMissileMk1GoodId)
		{
			assembler = &factory;
			break;
		}
	}

	REQUIRE(assembler != nullptr);
	assembler->productEfficiencyPerThousand = 800;
	assembler->remainingOutputTicks = 1;
	retargetFactoryRecipe(*assembler, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk2));
	REQUIRE(assembler->outputGoodId == PrototypeMissileMk2GoodId);
	REQUIRE(assembler->recipeId == FactoryRecipeId_PrototypeMissileMk2);
	REQUIRE(assembler->productEfficiencyPerThousand == 640);
	REQUIRE(assembler->remainingOutputTicks == 0);
	REQUIRE(assembler->productRampUnitsProduced == 0.f);
}

TEST_CASE("retarget Mk2 assembler back to Mk1 keeps most efficiency", "[slr][VariantOf]")
{
	Factory assembler = makeFactoryFromRecipe(
		AvonSystemId,
		HelionEmpireId,
		*factoryRecipeById(FactoryRecipeId_PrototypeMissileMk2));
	assembler.productEfficiencyPerThousand = 800;
	retargetFactoryRecipe(assembler, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1));
	REQUIRE(assembler.outputGoodId == PrototypeMissileMk1GoodId);
	REQUIRE(assembler.productEfficiencyPerThousand == 640);
}

TEST_CASE("retarget assembler to steel mill is square one", "[slr][VariantOf]")
{
	Factory assembler = makeFactoryFromRecipe(
		AvonSystemId,
		HelionEmpireId,
		*factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1));
	assembler.productEfficiencyPerThousand = 800;
	const FactoryRecipe mill{
		.inputsPerTick = { GoodQuantity{ .goodId = IronOreGoodId, .amount = FactoryInputOrePerTick } },
		.outputGoodId = SteelGoodId,
		.outputAmountPerTick = FactoryOutputSteelPerTick,
		.outputMaxStockpile = FactoryMaxStockpile_Steel };
	retargetFactoryRecipe(assembler, mill);
	REQUIRE(assembler.outputGoodId == SteelGoodId);
	REQUIRE(assembler.productEfficiencyPerThousand == FactoryStartEfficiencyPerThousand);
}

TEST_CASE("retarget same output good is a no-op", "[slr][VariantOf]")
{
	Factory assembler = makeFactoryFromRecipe(
		AvonSystemId,
		HelionEmpireId,
		*factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1));
	assembler.productEfficiencyPerThousand = 800;
	assembler.remainingOutputTicks = 1;
	retargetFactoryRecipe(assembler, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1));
	REQUIRE(assembler.productEfficiencyPerThousand == 800);
	REQUIRE(assembler.remainingOutputTicks == 1);
}

TEST_CASE("retargeted assembler emits one Mk2 missile after two ticks", "[slr][VariantOf]")
{
	using Catch::Approx;
	WorldData world;
	StarSystem avon{ .id = AvonSystemId, .name = "Avon" };
	avon.stocks.push_back(GoodStock{ .goodId = MissileEngineMk2GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk2GoodId, .goodAmount = 0.f });
	world.systems.push_back(avon);
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1)));
	retargetFactoryRecipe(world.factories[0], *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk2));
	world.factories[0].productEfficiencyPerThousand = FactoryFullEfficiencyPerThousand;

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk2GoodId) == Approx(0.f));
	REQUIRE(getStockpileAmount(world.systems[0], MissileEngineMk2GoodId) == Approx(0.f));

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk2GoodId) == Approx(1.f));
}

TEST_CASE("Mk2 assembler stalls without Mk2 engine", "[slr][VariantOf]")
{
	using Catch::Approx;
	WorldData world;
	StarSystem avon{ .id = AvonSystemId, .name = "Avon" };
	avon.stocks.push_back(GoodStock{ .goodId = MissileEngineMk2GoodId, .goodAmount = 0.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 1.f });
	avon.stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk2GoodId, .goodAmount = 0.f });
	world.systems.push_back(avon);
	world.factories.push_back(
		makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk2)));
	world.factories[0].productEfficiencyPerThousand = FactoryFullEfficiencyPerThousand;

	applyFactoryProduction(world);
	REQUIRE(getStockpileAmount(world.systems[0], PrototypeMissileMk2GoodId) == Approx(0.f));
	REQUIRE(world.factories[0].remainingOutputTicks == 0);
}

}
}
