#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/LaneMotion.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace JobTests
{

TEST_CASE("starter Helion haul job is uncovered steel amount, not a truck", "[slr][Job]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(uncoveredNeed(*helion, SteelGoodId) == 950);

	const HaulJob* job = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(job != nullptr);
	REQUIRE(job->destinationSystemId == 1);
	REQUIRE(job->goodId == SteelGoodId);
	REQUIRE(job->amount == uncoveredNeed(*helion, SteelGoodId));
	REQUIRE(job->amount == 950);
	REQUIRE(world.haulJobById(job->id) == job);
}

TEST_CASE("Rimward has no haul job", "[slr][Job]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(haulJobFor(world, 5, SteelGoodId) == nullptr);
}

TEST_CASE("uncovered need below a truckload removes the haul job", "[slr][Job]")
{
	WorldData world;
	world.systems = { StarSystem{ .id = 1, .name = "A", .position = { 0.0f, 0.0f } } };
	world.systems.front().stocks.push_back(GoodStock{
		.goodId = SteelGoodId,
		.need = TruckCapacity_Steel - 1 });
	syncHaulJobsFromUncoveredNeed(world);
	REQUIRE(haulJobFor(world, 1, SteelGoodId) == nullptr);
}

TEST_CASE("a convoy of 19 steel trucks can carry Helion's 950 in one go; two cannot", "[slr][Job]")
{
	REQUIRE(convoyCarriesAmountInOneGo(19, 950, SteelGoodId));
	REQUIRE_FALSE(convoyCarriesAmountInOneGo(2, 950, SteelGoodId));
}

TEST_CASE("150 steel needs three trucks in one go", "[slr][Job]")
{
	REQUIRE(truckCountToCarryAmountInOneGo(150, SteelGoodId) == 3);
}

TEST_CASE("two empty trucks accept the same Helion job and pathfind Rimward to Helion", "[slr][Job]")
{
	WorldData world = makeStarterMap();
	const HaulJob* jobBefore = haulJobFor(world, 1, SteelGoodId);
	REQUIRE(jobBefore != nullptr);
	const long long jobId = jobBefore->id;
	const long long amountBefore = jobBefore->amount;

	stepGalaxy(world);

	REQUIRE(world.trucks[0].haulJobId == jobId);
	REQUIRE(world.trucks[1].haulJobId == jobId);
	REQUIRE(world.trucks[0].motion.route == shortestLanePath(world, 5, 1));
	REQUIRE(world.trucks[1].motion.route == shortestLanePath(world, 5, 1));
	const HaulJob* job = world.haulJobById(jobId);
	REQUIRE(job != nullptr);
	REQUIRE(job->amount == amountBefore - 2 * TruckCapacity_Steel);
}

TEST_CASE("second accept drops when remaining room is one truckload", "[slr][Job]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	HaulJob* job = world.haulJobById(haulJobFor(world, 1, SteelGoodId)->id);
	REQUIRE(job != nullptr);
	job->amount = TruckCapacity_Steel;
	world.trucks.push_back(Truck{
		.id = 10,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .speed = 25.0f } });
	world.trucks.push_back(Truck{
		.id = 11,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .speed = 25.0f } });

	REQUIRE(execute(
		world,
		GameCommand::acceptHaulJob(10, job->id, CommandSource::Ai)));
	REQUIRE_FALSE(execute(
		world,
		GameCommand::acceptHaulJob(11, job->id, CommandSource::Ai)));
	REQUIRE(world.trucks[0].haulJobId == job->id);
	REQUIRE(world.trucks[1].haulJobId == 0);
	REQUIRE(job->amount == 0);
}

TEST_CASE("Helion truck does not accept an Ashreach haul job", "[slr][Job]")
{
	WorldData world = makeStarterMap();
	world.trucks.clear();
	world.haulJobs.clear();
	world.haulJobs.push_back(HaulJob{
		.id = 9,
		.destinationSystemId = 1,
		.empireId = AshreachEmpireId,
		.goodId = SteelGoodId,
		.amount = 500 });
	world.trucks.push_back(Truck{
		.id = 1,
		.empireId = HelionEmpireId,
		.motion = { .route = { 5 }, .speed = 25.0f } });

	REQUIRE_FALSE(execute(
		world,
		GameCommand::acceptHaulJob(1, 9, CommandSource::Ai)));
	REQUIRE(world.trucks[0].haulJobId == 0);
}

TEST_CASE("execute accept fails when the job is already full", "[slr][Job]")
{
	WorldData world = makeStarterMap();
	HaulJob* job = world.haulJobById(haulJobFor(world, 1, SteelGoodId)->id);
	REQUIRE(job != nullptr);
	job->amount = 0;
	REQUIRE_FALSE(execute(
		world,
		GameCommand::acceptHaulJob(1, job->id, CommandSource::Ai)));
}

}
}
