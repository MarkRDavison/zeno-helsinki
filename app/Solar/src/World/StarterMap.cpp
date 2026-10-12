#include <World/StarterMap.hpp>
#include <World/Goods.hpp>
#include <World/TruckCargo.hpp>
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

}

	WorldData makeStarterMap()
	{
		WorldData world;

		world.systems = {
			StarSystem{ .id = 1, .name = "Helion", .position = { 120.0f, 280.0f } },
			StarSystem{ .id = 2, .name = "Kestrel", .position = { 340.0f, 180.0f } },
			StarSystem{ .id = 3, .name = "Junction", .position = { 520.0f, 320.0f } },
			StarSystem{ .id = 4, .name = "Ashreach", .position = { 480.0f, 520.0f } },
			StarSystem{ .id = 5, .name = "Rimward", .position = { 760.0f, 140.0f } },
			StarSystem{ .id = 6, .name = "Caldera", .position = { 820.0f, 380.0f } },
			StarSystem{ .id = 7, .name = "Nadir", .position = { 980.0f, 260.0f } },
			StarSystem{ .id = 8, .name = "Vesper", .position = { 700.0f, 560.0f } },
		};

		addLane(world, 1, 2);
		addLane(world, 2, 3);
		addLane(world, 3, 4);
		addLane(world, 3, 5);
		addLane(world, 3, 6);
		addLane(world, 5, 7);
		addLane(world, 6, 7);
		addLane(world, 4, 8);
		addLane(world, 6, 8);

		StarSystem* rimward = world.systemById(5);
		rimward->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0, .need = 0 });

		StarSystem* helion = world.systemById(1);
		helion->stocks.push_back(GoodStock{ .goodId = SteelGoodId, .goodAmount = 0 });

		world.mines.push_back(Mine{
			.systemId = 5,
			.outputsPerTick = {
				MineOutput{
					.goodId = SteelGoodId,
					.amountPerTick = 100,
					.maxStockpile = MineMaxStockpile_Steel } } });
		world.yards.push_back(Yard{
			.systemId = 1,
			.desiredPiles = { GoodQuantity{ .goodId = SteelGoodId, .amount = 1000 } } });
		postNeedsFromConsumerShortfalls(world, *helion);
		postNeedsFromConsumerShortfalls(world, *rimward);

		constexpr float civilianTruckSpeed = 25.0f;
		const std::vector<long long> rimwardToHelion{ 5, 3, 2, 1 };
		world.trucks.push_back(Truck{
			.id = 1,
			.motion = {
				.route = rimwardToHelion,
				.hop = 0,
				.progress = 1.0f,
				.direction = 1,
				.speed = civilianTruckSpeed },
			.cargoGoodId = 0,
			.cargoAmount = 0 });
		world.trucks.push_back(Truck{
			.id = 2,
			.motion = {
				.route = rimwardToHelion,
				.hop = 0,
				.progress = 0.0f,
				.direction = 1,
				.speed = civilianTruckSpeed },
			.cargoGoodId = 0,
			.cargoAmount = 0 });
		world.trucks.push_back(Truck{
			.id = 3,
			.motion = {
				.route = rimwardToHelion,
				.hop = 0,
				.progress = 0.5f,
				.direction = 1,
				.speed = civilianTruckSpeed },
			.cargoGoodId = SteelGoodId,
			.cargoAmount = TruckCapacity_Steel });
		raiseInboundOnLoad(*helion, SteelGoodId, TruckCapacity_Steel);
		syncHaulJobsFromUncoveredNeed(world);

		world.empires = {
			Empire{ .id = HelionEmpireId, .name = "Helion" },
			Empire{ .id = AshreachEmpireId, .name = "Ashreach" },
		};

		const long long nikeId = 1;
		const long long destroyerId = 2;
		const long long lanceId = 3;
		const long long cutterId = 4;
		world.shipClasses.push_back(ShipClass{
			.id = nikeId,
			.empireId = HelionEmpireId,
			.name = "Nike",
			.range = 55.0f,
			.accuracy = 0.75f,
			.laserDamage = 8,
			.maxShields = 20,
			.maxHull = 28,
			.closeRate = 4.0f });
		world.shipClasses.push_back(ShipClass{
			.id = destroyerId,
			.empireId = HelionEmpireId,
			.name = "Destroyer",
			.range = 35.0f,
			.accuracy = 0.80f,
			.laserDamage = 4,
			.maxShields = 8,
			.maxHull = 12,
			.closeRate = 6.0f });
		world.shipClasses.push_back(ShipClass{
			.id = lanceId,
			.empireId = AshreachEmpireId,
			.name = "Lance",
			.range = 75.0f,
			.accuracy = 0.65f,
			.laserDamage = 7,
			.maxShields = 18,
			.maxHull = 24,
			.closeRate = 3.0f });
		world.shipClasses.push_back(ShipClass{
			.id = cutterId,
			.empireId = AshreachEmpireId,
			.name = "Cutter",
			.range = 30.0f,
			.accuracy = 0.78f,
			.laserDamage = 3,
			.maxShields = 6,
			.maxHull = 10,
			.closeRate = 7.0f });

		constexpr float homeFleetSpeed = 50.0f;
		world.fleets.push_back(Fleet{
			.id = 1,
			.empireId = HelionEmpireId,
			.name = "Home Fleet",
			.systemId = 1,
			.composition = {
				ClassCount{ .classId = nikeId, .count = 8 },
				ClassCount{ .classId = destroyerId, .count = 12 },
			},
			.motion = { .speed = homeFleetSpeed } });
		world.fleets.push_back(Fleet{
			.id = 2,
			.empireId = AshreachEmpireId,
			.name = "Kestrel Squadron",
			.systemId = 2,
			.composition = {
				ClassCount{ .classId = lanceId, .count = 6 },
				ClassCount{ .classId = cutterId, .count = 10 },
			},
			.motion = { .speed = homeFleetSpeed } });

		return world;
	}

}
