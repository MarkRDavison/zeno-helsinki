#include <World/StarterMap.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

namespace slr
{
namespace OwnershipTests
{

TEST_CASE("Helion is Helion-owned populated with score from yard and population", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* helion = world.systemById(1);
	REQUIRE(helion != nullptr);
	REQUIRE(helion->empireId == HelionEmpireId);
	REQUIRE(systemOwnershipType(world, 1) == SystemOwnershipType::Populated);
	REQUIRE(systemOwnershipScore(world, 1) > 0);
	REQUIRE(std::string(systemOwnershipTypeName(SystemOwnershipType::Populated)) == "populated");
}

TEST_CASE("Rimward is Helion-owned extraction", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* rimward = world.systemById(5);
	REQUIRE(rimward != nullptr);
	REQUIRE(rimward->empireId == HelionEmpireId);
	REQUIRE(systemOwnershipType(world, 5) == SystemOwnershipType::Extraction);
	REQUIRE(systemOwnershipScore(world, 5) == 1);
}

TEST_CASE("Junction is Helion-owned industrial", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* junction = world.systemById(3);
	REQUIRE(junction != nullptr);
	REQUIRE(junction->empireId == HelionEmpireId);
	REQUIRE(systemOwnershipType(world, 3) == SystemOwnershipType::Industrial);
	REQUIRE(systemOwnershipScore(world, 3) == 1);
}

TEST_CASE("Kestrel and Ashreach are Ashreach-claimed empty with score 0", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* kestrel = world.systemById(2);
	const StarSystem* ashreach = world.systemById(4);
	REQUIRE(kestrel != nullptr);
	REQUIRE(ashreach != nullptr);
	REQUIRE(kestrel->empireId == AshreachEmpireId);
	REQUIRE(ashreach->empireId == AshreachEmpireId);
	REQUIRE(systemOwnershipType(world, 2) == SystemOwnershipType::Empty);
	REQUIRE(systemOwnershipType(world, 4) == SystemOwnershipType::Empty);
	REQUIRE(systemOwnershipScore(world, 2) == 0);
	REQUIRE(systemOwnershipScore(world, 4) == 0);
}

TEST_CASE("Vesper is Ashreach-owned populated with mine and yard", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	const StarSystem* vesper = world.systemById(8);
	REQUIRE(vesper != nullptr);
	REQUIRE(vesper->empireId == AshreachEmpireId);
	REQUIRE(systemOwnershipType(world, 8) == SystemOwnershipType::Populated);
	REQUIRE(systemOwnershipScore(world, 8) > 0);
}

TEST_CASE("Caldera and Nadir are unowned empty", "[slr][Ownership]")
{
	const WorldData world = makeStarterMap();
	for (long long id : { 6, 7 })
	{
		const StarSystem* system = world.systemById(id);
		REQUIRE(system != nullptr);
		REQUIRE(system->empireId == 0);
		REQUIRE(systemOwnershipType(world, id) == SystemOwnershipType::Empty);
		REQUIRE(systemOwnershipScore(world, id) == 0);
	}
}

TEST_CASE("parked fleet adds presence not ownership score", "[slr][Ownership]")
{
	WorldData world = makeStarterMap();
	REQUIRE(systemPresenceScore(world, 1) == 1);
	REQUIRE(systemPresenceScore(world, 4) == 1);
	REQUIRE(systemPresenceScore(world, 6) == 0);
	REQUIRE_FALSE(world.fleets.empty());
	world.fleets.front().systemId = 6;
	REQUIRE(systemOwnershipScore(world, 6) == 0);
	REQUIRE(systemOwnershipType(world, 6) == SystemOwnershipType::Empty);
	REQUIRE(systemPresenceScore(world, 6) == 1);
	REQUIRE(systemPresenceScore(world, 1) == 0);
}

TEST_CASE("in-transit fleet does not add presence", "[slr][Ownership]")
{
	WorldData world = makeStarterMap();
	REQUIRE_FALSE(world.fleets.empty());
	world.fleets.front().motion.progress = 0.5f;
	REQUIRE(systemPresenceScore(world, 1) == 0);
}

}
}
