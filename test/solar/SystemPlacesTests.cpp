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

TEST_CASE("systems other than Helion have no layout", "[slr][SystemPlaces]")
{
	const WorldData world = makeStarterMap();
	REQUIRE(world.systemLayouts.size() == 1);
	for (const auto& system : world.systems)
	{
		if (system.id == HelionSystemId)
		{
			REQUIRE(world.systemLayoutBySystemId(system.id) != nullptr);
			continue;
		}

		REQUIRE(world.systemLayoutBySystemId(system.id) == nullptr);
	}
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
