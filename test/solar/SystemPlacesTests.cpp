#include <World/GameCommand.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/SystemPlaces.hpp>
#include <PutAtJump.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace SystemPlacesTests
{

TEST_CASE("Helion has a star, derived out radius, Kestrel jump, and yard place", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	const SystemLayout* helion = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(helion != nullptr);
	REQUIRE(helion->starSize == Catch::Approx(HelionStarSize));
	REQUIRE(helion->starSize > 0.0f);
	REQUIRE(outRadiusForLayout(*helion) == Catch::Approx(outRadiusFromStarSize(HelionStarSize)));
	REQUIRE(outRadiusForLayout(*helion) == Catch::Approx(OutRadiusPerStarSize * HelionStarSize));

	const SystemPlace* star = starPlaceForSystem(world, HelionSystemId);
	REQUIRE(star != nullptr);
	REQUIRE(star->kind == SystemPlaceKind::Star);
	REQUIRE(star->id == HelionStarPlaceId);

	REQUIRE(hasHyperlane(world, HelionSystemId, KestrelSystemId));
	const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, HelionSystemId, KestrelSystemId);
	REQUIRE(jump != nullptr);
	REQUIRE(jump->kind == SystemPlaceKind::Jump);
	REQUIRE(jump->neighborSystemId == KestrelSystemId);
	REQUIRE(jumpPlaceIsAtOrOutsideOutRadius(*helion, *jump));
	REQUIRE(glm::length(jump->position - star->position) == Catch::Approx(outRadiusForLayout(*helion)));

	const SystemPlace* yard = yardPlaceForSystem(world, HelionSystemId);
	REQUIRE(yard != nullptr);
	REQUIRE(yard->kind == SystemPlaceKind::Yard);
	REQUIRE(glm::length(yard->position - star->position) < outRadiusForLayout(*helion));
}

TEST_CASE("Junction has mill place and jumps for each incident lane", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	const SystemLayout* junction = world.systemLayoutBySystemId(JunctionSystemId);
	REQUIRE(junction != nullptr);
	REQUIRE(junction->starSize == Catch::Approx(HelionStarSize));

	const SystemPlace* star = starPlaceForSystem(world, JunctionSystemId);
	REQUIRE(star != nullptr);

	for (long long neighborId : { KestrelSystemId, 4LL, RimwardSystemId, 6LL })
	{
		REQUIRE(hasHyperlane(world, JunctionSystemId, neighborId));
		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, JunctionSystemId, neighborId);
		REQUIRE(jump != nullptr);
		REQUIRE(jumpPlaceIsAtOrOutsideOutRadius(*junction, *jump));
	}

	const SystemPlace* factory = factoryPlaceForSystem(world, JunctionSystemId);
	REQUIRE(factory != nullptr);
	REQUIRE(factory->kind == SystemPlaceKind::Factory);
	REQUIRE(factory->id == JunctionFactoryPlaceId);
	REQUIRE(glm::length(factory->position - star->position) < outRadiusForLayout(*junction));

	const SystemPlace* picked = pickSystemPlace(*junction, factory->position);
	REQUIRE(picked != nullptr);
	REQUIRE(picked->kind == SystemPlaceKind::Factory);
}

TEST_CASE("Rimward has mine place and jumps for each incident lane", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	const SystemLayout* rimward = world.systemLayoutBySystemId(RimwardSystemId);
	REQUIRE(rimward != nullptr);

	const SystemPlace* star = starPlaceForSystem(world, RimwardSystemId);
	REQUIRE(star != nullptr);

	for (long long neighborId : { JunctionSystemId, 7LL })
	{
		REQUIRE(hasHyperlane(world, RimwardSystemId, neighborId));
		const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, RimwardSystemId, neighborId);
		REQUIRE(jump != nullptr);
		REQUIRE(jumpPlaceIsAtOrOutsideOutRadius(*rimward, *jump));
	}

	const SystemPlace* mine = minePlaceForSystem(world, RimwardSystemId);
	REQUIRE(mine != nullptr);
	REQUIRE(mine->kind == SystemPlaceKind::Mine);
	REQUIRE(mine->id == RimwardMinePlaceId);
	REQUIRE(glm::length(mine->position - star->position) < outRadiusForLayout(*rimward));

	const SystemPlace* picked = pickSystemPlace(*rimward, mine->position);
	REQUIRE(picked != nullptr);
	REQUIRE(picked->kind == SystemPlaceKind::Mine);
}

TEST_CASE("Helion has no factory place; Kestrel has jumps and a factory", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systemLayouts.size() == world.systems.size());
	REQUIRE(factoryPlaceForSystem(world, HelionSystemId) == nullptr);
	REQUIRE(minePlaceForSystem(world, HelionSystemId) == nullptr);
	const SystemLayout* kestrel = world.systemLayoutBySystemId(KestrelSystemId);
	REQUIRE(kestrel != nullptr);
	REQUIRE(jumpPlaceForLaneNeighbor(world, KestrelSystemId, HelionSystemId) != nullptr);
	REQUIRE(jumpPlaceForLaneNeighbor(world, KestrelSystemId, JunctionSystemId) != nullptr);
	REQUIRE(jumpPlaceForLaneNeighbor(world, KestrelSystemId, AvonSystemId) != nullptr);
	const SystemPlace* factory = factoryPlaceForSystem(world, KestrelSystemId);
	REQUIRE(factory != nullptr);
	REQUIRE(factory->kind == SystemPlaceKind::Factory);
	REQUIRE(factory->id == KestrelFactoryPlaceId);
	REQUIRE(glm::length(factory->position - starPositionInSystemSpace(*kestrel)) < outRadiusForLayout(*kestrel));
	const SystemPlace* picked = pickSystemPlace(*kestrel, factory->position);
	REQUIRE(picked != nullptr);
	REQUIRE(picked->kind == SystemPlaceKind::Factory);
}

TEST_CASE("every starter system has a star and a jump per incident lane", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systemLayouts.size() == world.systems.size());
	for (const auto& system : world.systems)
	{
		const SystemLayout* layout = world.systemLayoutBySystemId(system.id);
		REQUIRE(layout != nullptr);
		REQUIRE(starPlaceForSystem(world, system.id) != nullptr);
		for (const auto& lane : world.hyperlanes)
		{
			long long neighborId = 0;
			if (lane.a == system.id)
			{
				neighborId = lane.b;
			}
			else if (lane.b == system.id)
			{
				neighborId = lane.a;
			}
			else
			{
				continue;
			}

			const SystemPlace* jump = jumpPlaceForLaneNeighbor(world, system.id, neighborId);
			REQUIRE(jump != nullptr);
			REQUIRE(jumpPlaceIsAtOrOutsideOutRadius(*layout, *jump));
		}
	}

	REQUIRE(yardPlaceForSystem(world, AshreachSystemId) == nullptr);
	REQUIRE(factoryPlaceForSystem(world, CalderaSystemId) == nullptr);
	REQUIRE(minePlaceForSystem(world, NadirSystemId) == nullptr);
	REQUIRE(factoryPlaceForSystem(world, AvonSystemId) != nullptr);
	REQUIRE(factoryPlaceForSystem(world, AvonSystemId)->id == AvonFactoryPlaceId);
	REQUIRE(yardPlaceForSystem(world, VesperSystemId) != nullptr);
	REQUIRE(yardPlaceForSystem(world, VesperSystemId)->id == VesperYardPlaceId);
	REQUIRE(minePlaceForSystem(world, VesperSystemId) != nullptr);
	REQUIRE(minePlaceForSystem(world, VesperSystemId)->id == VesperMinePlaceId);
	const Fleet* ashreach = world.fleetById(2);
	REQUIRE(ashreach != nullptr);
	const SystemPlace* ashreachJump =
		jumpPlaceForLaneNeighbor(world, AshreachSystemId, JunctionSystemId);
	REQUIRE(ashreachJump != nullptr);
	REQUIRE(ashreach->placeId == ashreachJump->id);
}

TEST_CASE("starter map still has Junction factory and Rimward mine rows", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	bool junctionHasFactory = false;
	for (const auto& factory : world.factories)
	{
		if (factory.systemId == JunctionSystemId)
		{
			junctionHasFactory = true;
			break;
		}
	}

	REQUIRE(junctionHasFactory);
	bool rimwardHasMine = false;
	for (const auto& mine : world.mines)
	{
		if (mine.systemId == RimwardSystemId)
		{
			rimwardHasMine = true;
			break;
		}
	}

	REQUIRE(rimwardHasMine);
}

TEST_CASE("Home Fleet is at the Helion yard; Kestrel Fleet is at the factory", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	const Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	REQUIRE(home->placeId == HelionYardPlaceId);
	REQUIRE(parkedPlaceInSystem(world, HelionSystemId, home->placeId) != nullptr);
	REQUIRE(parkedPlaceInSystem(world, HelionSystemId, home->placeId)->kind == SystemPlaceKind::Yard);

	const Fleet* kestrelFleet = world.fleetById(3);
	REQUIRE(kestrelFleet != nullptr);
	REQUIRE(kestrelFleet->placeId == KestrelFactoryPlaceId);
	REQUIRE(parkedPlaceInSystem(world, KestrelSystemId, kestrelFleet->placeId)->kind == SystemPlaceKind::Factory);
}

TEST_CASE("parked Rimward trucks sit at the mine; flying truck has no place", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	bool sawParkedMineTruck = false;
	bool sawFlyingWithoutPlace = false;
	for (const auto& truck : world.trucks)
	{
		if (isTruckAtSystem(truck, RimwardSystemId))
		{
			REQUIRE(truck.placeId == RimwardMinePlaceId);
			sawParkedMineTruck = true;
		}

		if (truck.motion.progress > 0.0f)
		{
			REQUIRE(truck.placeId == 0);
			sawFlyingWithoutPlace = true;
		}
	}

	REQUIRE(sawParkedMineTruck);
	REQUIRE(sawFlyingWithoutPlace);
}

TEST_CASE("MoveFleetToSystem from the Helion Kestrel jump clears place on a galaxy lane", "[slr][SystemPlaces]")
{
	WorldData world = makeStarterMap();
	const long long fleetId = 1;
	putFleetAtJumpToward(world, fleetId, KestrelSystemId);
	REQUIRE(execute(world, GameCommand::moveFleetToSystem(fleetId, KestrelSystemId, CommandSource::Player)));
	const Fleet* home = world.fleetById(fleetId);
	REQUIRE(home != nullptr);
	REQUIRE(home->motion.route.size() >= 2);
	REQUIRE(home->placeId == 0);
}

TEST_CASE("after an intermediate hop the truck crawls with a parked system id", "[slr][SystemPlaces]")
{
	WorldData world = makeStarterMap();
	const SystemPlace* kestrelJunctionJump =
		jumpPlaceForLaneNeighbor(world, KestrelSystemId, JunctionSystemId);
	REQUIRE(kestrelJunctionJump != nullptr);
	Truck truck{
		.id = 1,
		.motion = {
			.route = { JunctionSystemId, KestrelSystemId, HelionSystemId },
			.hop = 1,
			.progress = 0.0f,
			.direction = 1,
			.speed = 25.0f },
		.placeId = kestrelJunctionJump->id };
	long long parkedSystemId = KestrelSystemId;
	continueGalaxyPathAfterHop(
		world,
		parkedSystemId,
		truck.inSystem,
		truck.motion,
		truck.placeId,
		false);
	REQUIRE(inSystemMotionIsCrawling(truck.inSystem));
	REQUIRE(truckSystemId(truck) == KestrelSystemId);
	REQUIRE(truck.motion.route == std::vector<long long>{ KestrelSystemId });
}

TEST_CASE("truck at Rimward mine crawls to the Junction jump then FTL", "[slr][SystemPlaces]")
{
	WorldData world = makeStarterMap();
	Truck* parked = nullptr;
	for (auto& truck : world.trucks)
	{
		if (isTruckAtSystem(truck, RimwardSystemId) && truck.placeId == RimwardMinePlaceId)
		{
			parked = &truck;
			break;
		}
	}

	REQUIRE(parked != nullptr);
	const long long truckId = parked->id;
	REQUIRE(pathTruckToSystem(world, *parked, HelionSystemId));
	REQUIRE(inSystemMotionIsCrawling(parked->inSystem));
	REQUIRE(parked->motion.route.size() < 2);
	stepUntilTruckHasGalaxyRoute(world, truckId);
	parked = world.truckById(truckId);
	REQUIRE(parked != nullptr);
	REQUIRE(parked->motion.route.size() >= 2);
	const SystemPlace* outbound =
		jumpPlaceForLaneNeighbor(world, RimwardSystemId, JunctionSystemId);
	REQUIRE(outbound != nullptr);
	REQUIRE(parked->placeId == outbound->id);
	REQUIRE_FALSE(inSystemMotionIsCrawling(parked->inSystem));
}

TEST_CASE("a jump inside the out radius is rejected", "[slr][SystemPlaces]")
{
	SystemLayout layout;
	layout.starSize = 10.0f;
	layout.places.push_back(SystemPlace{
		.id = 1,
		.kind = SystemPlaceKind::Star,
		.position = { 0.0f, 0.0f } });
	const SystemPlace insideJump{
		.id = 2,
		.kind = SystemPlaceKind::Jump,
		.position = { 10.0f, 0.0f },
		.neighborSystemId = 2 };
	REQUIRE_FALSE(jumpPlaceIsAtOrOutsideOutRadius(layout, insideJump));
}

TEST_CASE("haul load and dump places follow mine then mill then yard", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	const SystemPlace* rimwardMine = haulLoadPlaceForGood(world, RimwardSystemId, IronOreGoodId);
	REQUIRE(rimwardMine != nullptr);
	REQUIRE(rimwardMine->id == RimwardMinePlaceId);

	const SystemPlace* junctionOreDump = haulDumpPlaceForGood(world, JunctionSystemId, IronOreGoodId);
	REQUIRE(junctionOreDump != nullptr);
	REQUIRE(junctionOreDump->id == JunctionFactoryPlaceId);

	const SystemPlace* junctionSteelLoad = haulLoadPlaceForGood(world, JunctionSystemId, SteelGoodId);
	REQUIRE(junctionSteelLoad != nullptr);
	REQUIRE(junctionSteelLoad->id == JunctionFactoryPlaceId);

	const SystemPlace* helionSteelDump = haulDumpPlaceForGood(world, HelionSystemId, SteelGoodId);
	REQUIRE(helionSteelDump != nullptr);
	REQUIRE(helionSteelDump->id == HelionYardPlaceId);
}

TEST_CASE("placeJumpOnOutRadiusTowardNeighbor lands on the ring", "[slr][SystemPlaces]")
{
	SystemLayout layout;
	layout.starSize = 10.0f;
	layout.places.push_back(SystemPlace{
		.id = 1,
		.kind = SystemPlaceKind::Star,
		.position = { 0.0f, 0.0f } });
	placeJumpOnOutRadiusTowardNeighbor(layout, glm::vec2{ 100.0f, 0.0f }, 2);
	const SystemPlace* jump = jumpPlaceForLaneNeighbor(layout, 2);
	REQUIRE(jump != nullptr);
	REQUIRE(jumpPlaceIsAtOrOutsideOutRadius(layout, *jump));
	REQUIRE(jump->position.x == Catch::Approx(outRadiusFromStarSize(10.0f)));
	REQUIRE(jump->position.y == Catch::Approx(0.0f));
}

}
}
