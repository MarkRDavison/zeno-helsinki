#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <World/TruckCargo.hpp>
#include <World/SystemPlaces.hpp>
#include <PutAtJump.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

namespace slr
{
namespace TruckTests
{

void clearAvonSteel(WorldData& world)
{
	StarSystem* avon = world.systemById(AvonSystemId);
	REQUIRE(avon != nullptr);
	for (auto& stock : avon->stocks)
	{
		if (stock.goodId == SteelGoodId)
		{
			stock.goodAmount = 0.f;
			return;
		}
	}
}

TEST_CASE("starter empty trucks park at Rimward; one loaded truck is Helion-bound", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(
		world.trucks.size() ==
		StarterCivilianTruckCount + StarterMilitaryTruckCount + StarterTenderCount);
	REQUIRE(world.trucks[0].motion.route == std::vector<long long>{ 5 });
	REQUIRE(world.trucks[1].motion.route == std::vector<long long>{ 5 });
	const Truck* flying = world.truckById(StarterCivilianTruckCount);
	REQUIRE(flying != nullptr);
	REQUIRE(flying->motion.route == shortestLanePath(world, 5, 1));
	REQUIRE(flying->cargoAmount == TruckCapacity_Steel);
	REQUIRE(flying->haulJobId != 0);
	long long militaryCount = 0;
	for (const auto& truck : world.trucks)
	{
		if (truckHasRole(world, truck, HullRole::MilitaryCargo))
		{
			++militaryCount;
			REQUIRE(truck.motion.route == std::vector<long long>{ 1 });
		}
	}
	REQUIRE(militaryCount == StarterMilitaryTruckCount);
}

TEST_CASE("starter truck speeds vary by 5 percent around 25", "[slr][Truck]")
{
	const WorldData a = makeStarterMap();
	const WorldData b = makeStarterMap();
	REQUIRE(
		a.trucks.size() ==
		StarterCivilianTruckCount + StarterMilitaryTruckCount + StarterTenderCount);
	REQUIRE(
		b.trucks.size() ==
		StarterCivilianTruckCount + StarterMilitaryTruckCount + StarterTenderCount);
	constexpr float base = 25.0f;
	constexpr float minSpeed = base * 0.95f;
	constexpr float maxSpeed = base * 1.05f;
	bool anyDifferent = false;
	for (size_t i = 0; i < a.trucks.size(); ++i)
	{
		const float speed = a.trucks[i].motion.speed;
		REQUIRE(speed >= minSpeed);
		REQUIRE(speed < maxSpeed);
		REQUIRE(speed == b.trucks[i].motion.speed);
		if (speed != a.trucks[0].motion.speed)
		{
			anyDifferent = true;
		}
	}
	REQUIRE(anyDifferent);
}

TEST_CASE("Helion starts with steel need 1200 and Rimward ore need is 0", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	const StarSystem* rimward = world.systemById(5);
	REQUIRE(helion != nullptr);
	REQUIRE(rimward != nullptr);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1200);
	REQUIRE(getNeed(*rimward, IronOreGoodId) == 0);
	REQUIRE(getInbound(*helion, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(getInbound(*rimward, IronOreGoodId) == 0);
	long long cargoTowardHelion = 0;
	for (const auto& truck : world.trucks)
	{
		if (truck.cargoAmount > 0)
		{
			cargoTowardHelion += truck.cargoAmount;
		}
	}
	REQUIRE(getInbound(*helion, SteelGoodId) == cargoTowardHelion);
}

TEST_CASE("trucks at a system are those sitting on the node", "[slr][Truck]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(countTrucksAtSystem(world, 5) == StarterCivilianTruckCount - 1);
	REQUIRE(countTrucksAtSystem(world, 1) == StarterMilitaryTruckCount + StarterTenderCount);
	REQUIRE(countTrucksAtSystem(world, 3) == 0);
}

TEST_CASE("after enough steps a truck is in progress on a hop", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	for (int i = 0; i < 4; ++i)
	{
		stepGalaxy(world);
	}

	bool onLane = false;
	for (const auto& truck : world.trucks)
	{
		if (truck.motion.progress > 0.0f && truck.motion.progress < 1.0f)
		{
			onLane = true;
		}
	}

	REQUIRE(onLane);
}

TEST_CASE("truck hop progress this tick is speed over lane length", "[slr][Truck]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 200.0f, 0.0f } },
	};
	world.hyperlanes.push_back(Hyperlane{ .a = 1, .b = 2 });
	world.trucks.push_back(Truck{
		.id = 1,
		.motion = {
			.route = { 1, 2 },
			.hop = 0,
			.progress = 0.0f,
			.direction = 1,
			.speed = 50.0f },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].motion.progress == Catch::Approx(0.25f));
	REQUIRE(world.trucks[0].motion.hop == 0);
}

TEST_CASE("truck loads ore at Rimward for the Junction mill job", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	world.trucks.clear();
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = RimwardMinePlaceId,
		.cargoGoodId = 0,
		.cargoAmount = 0,
		.classId = HelionCivilianTruckClassId });

	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	const StarSystem* junction = world.systemById(3);
	REQUIRE(rimward != nullptr);
	REQUIRE(junction != nullptr);
	REQUIRE(getStockpileAmount(*rimward, IronOreGoodId) == 50);
	REQUIRE(getInbound(*junction, IronOreGoodId) == TruckCapacity_IronOre);
	REQUIRE(getNeed(*junction, IronOreGoodId) == 1000);
	REQUIRE(world.trucks[0].cargoGoodId == IronOreGoodId);
	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_IronOre);
	REQUIRE(world.trucks[0].haulJobId != 0);
	stepUntilTruckHasGalaxyRoute(world, 1);
	REQUIRE(world.trucks[0].motion.route == shortestLanePath(world, 5, 3));
}

TEST_CASE("truck dump raises Helion stockpile and lowers need", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.mines.clear();
	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	world.yards.clear();
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = HelionYardPlaceId,
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = job->id,
		.classId = HelionCivilianTruckClassId });

	stepGalaxy(world);

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(getInbound(*helion, SteelGoodId) == 0);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(world.trucks[0].haulJobId == 0);
	REQUIRE(world.trucks[0].placeId == HelionYardPlaceId);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1150);
}

TEST_CASE("loaded steel truck crawls Kestrel between Junction and Helion jumps", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.convoys.clear();
	world.mines.clear();
	world.yards.clear();
	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	const SystemPlace* junctionKestrelJump =
		jumpPlaceForLaneNeighbor(world, JunctionSystemId, KestrelSystemId);
	REQUIRE(junctionKestrelJump != nullptr);
	world.convoys.push_back(Convoy{
		.id = 1,
		.haulJobId = job->id,
		.empireId = HelionEmpireId,
		.loadSystemId = JunctionSystemId,
		.motion = {
			.route = { JunctionSystemId, KestrelSystemId, HelionSystemId },
			.hop = 0,
			.progress = 0.0f,
			.direction = 1,
			.speed = 25.0f } });
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = {
			.route = { JunctionSystemId, KestrelSystemId, HelionSystemId },
			.hop = 0,
			.progress = 0.0f,
			.direction = 1,
			.speed = 25.0f },
		.placeId = junctionKestrelJump->id,
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = job->id,
		.convoyId = 1,
		.classId = HelionCivilianTruckClassId });

	const SystemPlace* kestrelJunctionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, JunctionSystemId);
	const SystemPlace* kestrelHelionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, HelionSystemId);
	REQUIRE(kestrelJunctionJump != nullptr);
	REQUIRE(kestrelHelionJump != nullptr);

	bool crawledKestrel = false;
	for (int i = 0; i < 256; ++i)
	{
		stepGalaxy(world);
		if (isTruckAtSystem(world.trucks[0], KestrelSystemId) &&
			inSystemMotionIsCrawling(world.trucks[0].inSystem) &&
			world.trucks[0].inSystem.fromPlaceId == kestrelJunctionJump->id &&
			world.trucks[0].inSystem.toPlaceId == kestrelHelionJump->id)
		{
			crawledKestrel = true;
			REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_Steel);
			REQUIRE(getStockpileAmount(*world.systemById(HelionSystemId), SteelGoodId) == 0);
			REQUIRE_FALSE(convoyIsUnderway(*world.convoyById(1)));
		}

		if (world.trucks[0].cargoAmount == 0)
		{
			break;
		}
	}

	REQUIRE(crawledKestrel);
	REQUIRE(world.trucks[0].placeId == HelionYardPlaceId);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(getStockpileAmount(*world.systemById(HelionSystemId), SteelGoodId) == 0);
	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*world.systemById(HelionSystemId), SteelGoodId) == TruckCapacity_Steel);
}

TEST_CASE("steel at the Helion Kestrel jump does not enter stockpile that tick", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.mines.clear();
	world.yards.clear();
	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = HelionKestrelJumpPlaceId,
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = job->id,
		.classId = HelionCivilianTruckClassId });

	stepGalaxy(world);

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_Steel);
	REQUIRE(inSystemMotionIsCrawling(world.trucks[0].inSystem));

	stepUntilTruckAtPlace(world, 1, HelionYardPlaceId);
	REQUIRE(world.trucks[0].placeId == HelionYardPlaceId);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);

	stepGalaxy(world);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == TruckCapacity_Steel);
}

TEST_CASE("empty truck at Rimward Junction jump does not load ore", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	world.trucks.clear();
	world.convoys.clear();
	StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	rimward->stocks[0].goodAmount = TruckCapacity_IronOre;
	const HaulJob* job = haulJobFor(world, 3, IronOreGoodId);
	REQUIRE(job != nullptr);
	const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, RimwardSystemId, JunctionSystemId);
	REQUIRE(jump != nullptr);
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.placeId = jump->id,
		.classId = HelionCivilianTruckClassId });

	REQUIRE(execute(world, GameCommand::acceptHaulJob(1, job->id, CommandSource::Ai)));
	stepGalaxy(world);

	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(getStockpileAmount(*rimward, IronOreGoodId) == TruckCapacity_IronOre + 100.f);
}

TEST_CASE("empty truck at Rimward does not load when mill and yard needs are 0", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	clearAvonSteel(world);
	world.trucks.clear();
	StarSystem* helion = world.systemById(1);
	StarSystem* junction = world.systemById(3);
	REQUIRE(helion != nullptr);
	REQUIRE(junction != nullptr);
	helion->stocks[0].need = 0;
	junction->stocks[0].need = 0;

	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.cargoGoodId = 0,
		.cargoAmount = 0,
		.classId = HelionCivilianTruckClassId });

	stepGalaxy(world);

	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(getStockpileAmount(*rimward, IronOreGoodId) == 100);
	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(world.trucks[0].haulJobId == 0);
	REQUIRE(world.trucks[0].motion.progress == 0.0f);
	REQUIRE(world.trucks[0].motion.hop == 0);
}

TEST_CASE("empty truck at Rimward does not load when uncovered need is below a truckload", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	StarSystem* helion = world.systemById(1);
	StarSystem* junction = world.systemById(3);
	REQUIRE(helion != nullptr);
	REQUIRE(junction != nullptr);
	helion->stocks[0].need = TruckCapacity_Steel;
	helion->stocks[0].inbound = TruckCapacity_Steel;
	junction->stocks[0].need = TruckCapacity_IronOre;
	junction->stocks[0].inbound = TruckCapacity_IronOre;

	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.cargoGoodId = 0,
		.cargoAmount = 0,
		.classId = HelionCivilianTruckClassId });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(getInbound(*helion, SteelGoodId) == TruckCapacity_Steel);
}

TEST_CASE("destroy loaded truck lowers dest inbound and leaves stockpile and need", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	const StarSystem* helionBefore = world.systemById(1);
	REQUIRE(helionBefore != nullptr);
	REQUIRE(getInbound(*helionBefore, SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(destroyTruckAndReduceInbound(world, StarterCivilianTruckCount));
	REQUIRE(std::none_of(
		world.trucks.begin(),
		world.trucks.end(),
		[](const Truck& truck) { return truck.id == StarterCivilianTruckCount; }));

	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getInbound(*helion, SteelGoodId) == 0);
	REQUIRE(getStockpileAmount(*helion, SteelGoodId) == 0);
	REQUIRE(getNeed(*helion, SteelGoodId) == 1200);
}

TEST_CASE("destroy empty truck does not change inbound", "[slr][Truck]")
{
	WorldData world = makeStarterMap();
	REQUIRE(destroyTruckAndReduceInbound(world, 2));
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(getInbound(*helion, SteelGoodId) == TruckCapacity_Steel);
}

TEST_CASE("truck without a system layout still dumps in-system this tick and credits stockpile next tick", "[slr][Truck]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.goodAmount = 0,
		.inbound = TruckCapacity_Steel,
		.need = 1000 });
	world.haulJobs.push_back(HaulJob{
		.id = 1,
		.destinationSystemId = 1,
		.empireId = 1,
		.goodId = SteelGoodId,
		.amount = TruckCapacity_Steel });
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = 1,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.cargoGoodId = SteelGoodId,
		.cargoAmount = TruckCapacity_Steel,
		.haulJobId = 1 });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].cargoAmount == 0);
	REQUIRE(getInbound(world.systems.front(), SteelGoodId) == 0);
	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == 0);

	stepGalaxy(world);

	REQUIRE(getStockpileAmount(world.systems.front(), SteelGoodId) == TruckCapacity_Steel);
	REQUIRE(getNeed(world.systems.front(), SteelGoodId) == 1000 - TruckCapacity_Steel);
}

TEST_CASE("truck without a system layout still loads in-system", "[slr][Truck]")
{
	WorldData world;
	world.systems = {
		StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } },
		StarSystem{ .id = 2, .name = "B", .position = { 200.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = IronOreGoodId,
		.goodAmount = TruckCapacity_IronOre });
	world.systems[1].stocks.push_back(GoodStock{ .goodId = IronOreGoodId });
	world.haulJobs.push_back(HaulJob{
		.id = 1,
		.destinationSystemId = 2,
		.empireId = 1,
		.goodId = IronOreGoodId,
		.amount = TruckCapacity_IronOre });
	world.convoys.push_back(Convoy{
		.id = 1,
		.haulJobId = 1,
		.empireId = 1,
		.loadSystemId = 1 });
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = 1,
		.motion = { .route = { 1 }, .hop = 0, .progress = 0.0f, .direction = 1, .speed = 25.0f },
		.haulJobId = 1,
		.convoyId = 1 });

	stepGalaxy(world);

	REQUIRE(world.trucks[0].cargoAmount == TruckCapacity_IronOre);
	REQUIRE(getStockpileAmount(world.systems.front(), IronOreGoodId) == 0);
}

}
}
