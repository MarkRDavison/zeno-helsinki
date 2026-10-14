#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/SystemPlaces.hpp>
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

TEST_CASE("Helion has no factory place; Kestrel has no layout", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systemLayouts.size() == 3);
	REQUIRE(factoryPlaceForSystem(world, HelionSystemId) == nullptr);
	REQUIRE(minePlaceForSystem(world, HelionSystemId) == nullptr);
	REQUIRE(world.systemLayoutBySystemId(KestrelSystemId) == nullptr);
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
