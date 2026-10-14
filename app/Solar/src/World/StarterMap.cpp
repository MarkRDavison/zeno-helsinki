#include <World/StarterMap.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/ShipDesign.hpp>
#include <World/TruckCargo.hpp>
#include <helsinki/System/Utils/SplitMix64.hpp>
#include <utility>

namespace slr
{
namespace
{

	void addLane(WorldData& world, long long a, long long b)
	{
		if (a > b)
		{
			std::swap(a, b);
		}

		world.hyperlanes.push_back(Hyperlane{ .a = a, .b = b });
	}

	float starterTruckSpeed(const WorldData& world, long long truckId)
	{
		constexpr float baseSpeed = 25.0f;
		constexpr std::uint64_t truckSpeedDomain = 0x5452554B535044ULL;
		std::uint64_t rng = hl::splitMix64Seed({
			world.masterSeed,
			truckSpeedDomain,
			static_cast<std::uint64_t>(truckId) });
		return baseSpeed * (0.95f + 0.10f * hl::splitMix64Unit(rng));
	}

}

	WorldData makeStarterMap()
	{
		WorldData world;

		world.systems = {
			StarSystem{ .id = 1, .name = "Helion", .position = { 120.0f, 280.0f }, .empireId = HelionEmpireId },
			StarSystem{ .id = 2, .name = "Kestrel", .position = { 340.0f, 180.0f }, .empireId = HelionEmpireId },
			StarSystem{ .id = 3, .name = "Junction", .position = { 520.0f, 320.0f }, .empireId = HelionEmpireId },
			StarSystem{ .id = 4, .name = "Ashreach", .position = { 480.0f, 520.0f }, .empireId = AshreachEmpireId },
			StarSystem{ .id = 5, .name = "Rimward", .position = { 760.0f, 140.0f }, .empireId = HelionEmpireId },
			StarSystem{ .id = 6, .name = "Caldera", .position = { 820.0f, 380.0f } },
			StarSystem{ .id = 7, .name = "Nadir", .position = { 980.0f, 260.0f } },
			StarSystem{ .id = 8, .name = "Vesper", .position = { 700.0f, 560.0f }, .empireId = AshreachEmpireId },
			StarSystem{ .id = AvonSystemId, .name = "Avon", .position = { 340.0f, 90.0f }, .empireId = HelionEmpireId },
		};

		addLane(world, 1, 2);
		addLane(world, 2, AvonSystemId);
		addLane(world, 2, 3);
		addLane(world, 3, 4);
		addLane(world, 3, 5);
		addLane(world, 3, 6);
		addLane(world, 5, 7);
		addLane(world, 6, 7);
		addLane(world, 4, 8);
		addLane(world, 6, 8);

		StarSystem* rimward = world.systemById(5);
		rimward->stocks.push_back(GoodStock{ .goodId = IronOreGoodId, .goodAmount = 0, .need = 0 });

		StarSystem* junction = world.systemById(3);
		junction->stocks.push_back(GoodStock{ .goodId = IronOreGoodId, .goodAmount = 0 });
		junction->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });

		StarSystem* kestrel = world.systemById(2);
		kestrel->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });
		kestrel->stocks.push_back(GoodStock{ .goodId = MissileGoodId, .goodAmount = 0 });

		StarSystem* helion = world.systemById(1);
		helion->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });
		helion->stocks.push_back(GoodStock{ .goodId = MissileGoodId, .goodAmount = YardStarterMissileStockpile });
		helion->stocks.push_back(GoodStock{
			.goodId = MissileLongRangeGoodId,
			.goodAmount = YardStarterLongRangeMissileStockpile });

		StarSystem* vesper = world.systemById(8);
		vesper->stocks.push_back(GoodStock{ .goodId = IronOreGoodId, .goodAmount = 0, .need = 0 });
		vesper->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });
		vesper->stocks.push_back(GoodStock{ .goodId = MissileGoodId, .goodAmount = YardStarterMissileStockpile });
		vesper->stocks.push_back(GoodStock{
			.goodId = MissileLongRangeGoodId,
			.goodAmount = YardStarterLongRangeMissileStockpile });

		StarSystem* avon = world.systemById(AvonSystemId);
		avon->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = AvonStarterSteelStockpile });
		avon->stocks.push_back(GoodStock{ .goodId = MissileEngineMk1GoodId, .goodAmount = 0 });
		avon->stocks.push_back(GoodStock{ .goodId = MissileWarheadMk1GoodId, .goodAmount = 0 });
		avon->stocks.push_back(GoodStock{ .goodId = MissileSeekerMk1GoodId, .goodAmount = 0 });
		avon->stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk1GoodId, .goodAmount = 0 });
		avon->stocks.push_back(GoodStock{ .goodId = MissileEngineMk2GoodId, .goodAmount = 0 });
		avon->stocks.push_back(GoodStock{ .goodId = PrototypeMissileMk2GoodId, .goodAmount = 0 });

		world.mines.push_back(Mine{
			.systemId = 5,
			.outputsPerTick = {
				MineOutput{
					.goodId = IronOreGoodId,
					.amountPerTick = 100.f,
					.maxStockpile = MineMaxStockpile_IronOre } } });
		world.factories.push_back(Factory{
			.systemId = 3,
			.empireId = HelionEmpireId,
			.desiredPiles = { GoodQuantity{ .goodId = IronOreGoodId, .amount = 1000.f } },
			.inputsPerTick = { GoodQuantity{ .goodId = IronOreGoodId, .amount = FactoryInputOrePerTick } },
			.outputGoodId = SteelGoodId,
			.outputAmountPerTick = FactoryOutputSteelPerTick,
			.outputMaxStockpile = FactoryMaxStockpile_Steel });
		world.factories.push_back(Factory{
			.systemId = 2,
			.empireId = HelionEmpireId,
			.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryMissileSteelDesiredPile } },
			.inputsPerTick = { GoodQuantity{ .goodId = SteelGoodId, .amount = FactoryMissileInputSteelPerTick } },
			.outputGoodId = MissileGoodId,
			.outputAmountPerTick = FactoryMissileOutputPerTick,
			.outputMaxStockpile = FactoryMaxStockpile_Missiles,
			.productEfficiencyPerThousand = FactoryStartEfficiencyPerThousand });
		world.factories.push_back(
			makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileEngineMk1)));
		world.factories.push_back(
			makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileWarheadMk1)));
		world.factories.push_back(
			makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileSeekerMk1)));
		world.factories.push_back(
			makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_PrototypeMissileMk1)));
		world.factories.push_back(
			makeFactoryFromRecipe(AvonSystemId, HelionEmpireId, *factoryRecipeById(FactoryRecipeId_MissileEngineMk2)));
		world.yards.push_back(Yard{
			.systemId = 1,
			.empireId = HelionEmpireId,
			.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = 1000.f } },
			.populationConsumePerTick = {
				GoodQuantity{ .goodId = SteelGoodId, .amount = YardPopulationConsumePerTick_Steel } },
			.populationReservedPiles = {
				GoodQuantity{ .goodId = SteelGoodId, .amount = YardPopulationReserved_Steel } },
			.slips = { YardSlip{ .id = 1, .remainingBuildTicks = 0 } } });
		world.mines.push_back(Mine{
			.systemId = 8,
			.outputsPerTick = {
				MineOutput{
					.goodId = IronOreGoodId,
					.amountPerTick = 100.f,
					.maxStockpile = MineMaxStockpile_IronOre } } });
		world.yards.push_back(Yard{
			.systemId = 8,
			.empireId = AshreachEmpireId,
			.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = 1000.f } },
			.populationConsumePerTick = {
				GoodQuantity{ .goodId = SteelGoodId, .amount = YardPopulationConsumePerTick_Steel } },
			.populationReservedPiles = {
				GoodQuantity{ .goodId = SteelGoodId, .amount = YardPopulationReserved_Steel } },
			.slips = { YardSlip{ .id = 2, .remainingBuildTicks = 0 } } });
		postNeedsFromConsumerShortfalls(world, *helion);
		postNeedsFromConsumerShortfalls(world, *kestrel);
		postNeedsFromConsumerShortfalls(world, *junction);
		postNeedsFromConsumerShortfalls(world, *rimward);
		postNeedsFromConsumerShortfalls(world, *vesper);

		for (long long truckId = 1; truckId < StarterCivilianTruckCount; ++truckId)
		{
			world.trucks.push_back(Truck{
				.id = truckId,
				.empireId = HelionEmpireId,
				.motion = {
					.route = { 5 },
					.hop = 0,
					.progress = 0.0f,
					.direction = 1,
					.speed = starterTruckSpeed(world, truckId) },
				.classId = HelionCivilianTruckClassId });
		}
		world.trucks.push_back(Truck{
			.id = StarterCivilianTruckCount,
			.empireId = HelionEmpireId,
			.motion = {
				.route = shortestLanePath(world, 5, 1),
				.hop = 0,
				.progress = 0.5f,
				.direction = 1,
				.speed = starterTruckSpeed(world, StarterCivilianTruckCount) },
			.cargoGoodId = SteelGoodId,
			.cargoAmount = TruckCapacity_Steel,
			.classId = HelionCivilianTruckClassId });
		raiseInboundOnLoad(*helion, SteelGoodId, TruckCapacity_Steel);
		syncHaulJobsFromUncoveredNeed(world);
		if (const HaulJob* job = haulJobFor(world, 1, SteelGoodId))
		{
			Truck* flying = world.truckById(StarterCivilianTruckCount);
			flying->haulJobId = job->id;
			world.convoys.push_back(Convoy{
				.id = 1,
				.haulJobId = job->id,
				.empireId = HelionEmpireId,
				.loadSystemId = 5,
				.motion = flying->motion });
			flying->convoyId = world.convoys.back().id;
		}

		for (long long i = 0; i < StarterMilitaryTruckCount; ++i)
		{
			const long long truckId = StarterCivilianTruckCount + 1 + i;
			world.trucks.push_back(Truck{
				.id = truckId,
				.empireId = HelionEmpireId,
				.motion = {
					.route = { 1 },
					.hop = 0,
					.progress = 0.0f,
					.direction = 1,
					.speed = starterTruckSpeed(world, truckId) },
				.classId = HelionMilitaryTruckClassId });
		}

		for (long long i = 0; i < StarterTenderCount; ++i)
		{
			const long long truckId =
				StarterCivilianTruckCount + StarterMilitaryTruckCount + 1 + i;
			world.trucks.push_back(Truck{
				.id = truckId,
				.empireId = HelionEmpireId,
				.motion = {
					.route = { 1 },
					.hop = 0,
					.progress = 0.0f,
					.direction = 1,
					.speed = starterTruckSpeed(world, truckId) },
				.classId = HelionTenderClassId });
		}

		world.engagementPolicies = {
			EngagementPolicy{
				.id = HelionHoldFirePolicyId,
				.empireId = HelionEmpireId,
				.name = "Hold fire",
				.ruleOfEngagement = RuleOfEngagement::DoNotFireUnlessFiredUpon },
			EngagementPolicy{
				.id = AshreachHoldFirePolicyId,
				.empireId = AshreachEmpireId,
				.name = "Hold fire",
				.ruleOfEngagement = RuleOfEngagement::DoNotFireUnlessFiredUpon },
			EngagementPolicy{
				.id = HelionWeaponsFreePolicyId,
				.empireId = HelionEmpireId,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
			EngagementPolicy{
				.id = AshreachWeaponsFreePolicyId,
				.empireId = AshreachEmpireId,
				.name = "Weapons free",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen },
			EngagementPolicy{
				.id = HelionHoldRangePolicyId,
				.empireId = HelionEmpireId,
				.name = "Hold range",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::HoldMissileRange },
			EngagementPolicy{
				.id = AshreachHoldRangePolicyId,
				.empireId = AshreachEmpireId,
				.name = "Hold range",
				.ruleOfEngagement = RuleOfEngagement::FireIfEnemySeen,
				.approach = FleetApproach::HoldMissileRange },
		};
		world.empires = {
			Empire{ .id = HelionEmpireId, .name = "Helion", .defaultPolicyId = HelionHoldFirePolicyId },
			Empire{ .id = AshreachEmpireId, .name = "Ashreach", .defaultPolicyId = AshreachHoldFirePolicyId },
		};

		const long long nikeId = 1;
		const long long destroyerId = 2;
		const long long lanceId = 3;
		const long long cutterId = 4;
		world.shipClasses.push_back(makeWarshipClassFromDraft(
			nikeId,
			HelionEmpireId,
			ShipClassDraft{
				.name = "Nike",
				.hullSize = HullSize::Small,
				.parts = {
					.reactorCount = 1,
					.engineCount = 1,
					.laserCount = 2,
					.missileTubeCount = 2,
					.cargoCount = 2 } }));
		world.shipClasses.push_back(makeWarshipClassFromDraft(
			destroyerId,
			HelionEmpireId,
			ShipClassDraft{
				.name = "Destroyer",
				.hullSize = HullSize::Small,
				.parts = {
					.reactorCount = 1,
					.engineCount = 2,
					.laserCount = 1,
					.missileTubeCount = 2,
					.cargoCount = 2 } }));
		world.shipClasses.push_back(ShipClass{
			.id = lanceId,
			.empireId = AshreachEmpireId,
			.name = "Lance",
			.range = 75.0f,
			.accuracy = 0.65f,
			.laserDamage = 7,
			.maxShields = 18,
			.maxHull = 24,
			.closeRate = 3.0f,
			.hullRoles = hullRoleBit(HullRole::Warship),
			.hullSize = HullSize::Medium,
			.missileTubes = 3,
			.missileReloadInnerTicks = 4,
			.magazineSize = 20,
			.missileDamage = 9,
			.missileRange = 120.0f,
			.missileSpeed = 28.0f });
		world.shipClasses.push_back(ShipClass{
			.id = cutterId,
			.empireId = AshreachEmpireId,
			.name = "Cutter",
			.range = 30.0f,
			.accuracy = 0.78f,
			.laserDamage = 3,
			.maxShields = 6,
			.maxHull = 10,
			.closeRate = 7.0f,
			.hullRoles = hullRoleBit(HullRole::Warship),
			.hullSize = HullSize::Medium,
			.missileTubes = 1,
			.missileReloadInnerTicks = 2,
			.magazineSize = 12,
			.missileDamage = 4,
			.missileRange = 50.0f,
			.missileSpeed = 20.0f });
		world.shipClasses.push_back(ShipClass{
			.id = HelionCivilianTruckClassId,
			.empireId = HelionEmpireId,
			.name = "Civilian truck",
			.hullRoles = hullRoleBit(HullRole::CivilianCargo),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });
		world.shipClasses.push_back(ShipClass{
			.id = HelionMilitaryTruckClassId,
			.empireId = HelionEmpireId,
			.name = "Military truck",
			.hullRoles = hullRoleBit(HullRole::MilitaryCargo),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });
		world.shipClasses.push_back(ShipClass{
			.id = AshreachCivilianTruckClassId,
			.empireId = AshreachEmpireId,
			.name = "Civilian truck",
			.hullRoles = hullRoleBit(HullRole::CivilianCargo),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });
		world.shipClasses.push_back(ShipClass{
			.id = AshreachMilitaryTruckClassId,
			.empireId = AshreachEmpireId,
			.name = "Military truck",
			.hullRoles = hullRoleBit(HullRole::MilitaryCargo),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });
		world.shipClasses.push_back(ShipClass{
			.id = HelionTenderClassId,
			.empireId = HelionEmpireId,
			.name = "Tender",
			.hullRoles = hullRoleBit(HullRole::Tender),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });
		world.shipClasses.push_back(ShipClass{
			.id = AshreachTenderClassId,
			.empireId = AshreachEmpireId,
			.name = "Tender",
			.hullRoles = hullRoleBit(HullRole::Tender),
			.steelCost = TruckHullSteelCost,
			.buildTicks = TruckHullBuildTicks });

		constexpr float homeFleetSpeed = 50.0f;
		world.fleets.push_back(Fleet{
			.id = 1,
			.empireId = HelionEmpireId,
			.name = "Home Fleet",
			.systemId = 1,
			.motion = { .speed = homeFleetSpeed } });
		appendFullMagazineHulls(world, world.fleets.back(), nikeId, 8);
		appendFullMagazineHulls(world, world.fleets.back(), destroyerId, 12);
		world.fleets.push_back(Fleet{
			.id = 3,
			.empireId = HelionEmpireId,
			.name = "Kestrel Fleet",
			.systemId = 2,
			.motion = { .speed = homeFleetSpeed } });
		appendEmptyMagazineHulls(world, world.fleets.back(), nikeId, 4);
		appendEmptyMagazineHulls(world, world.fleets.back(), destroyerId, 4);
		world.fleets.push_back(Fleet{
			.id = 2,
			.empireId = AshreachEmpireId,
			.name = "Ashreach Fleet",
			.systemId = 4,
			.motion = { .speed = homeFleetSpeed } });
		appendFullMagazineHulls(world, world.fleets.back(), lanceId, 6);
		appendFullMagazineHulls(world, world.fleets.back(), cutterId, 10);

		return world;
	}

}
