#include <Views/MainView.hpp>
#include <World/Goods.hpp>
#include <World/StarterMap.hpp>
#include <World/SystemPlaces.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace slr
{
namespace SystemViewTests
{

TEST_CASE("main view starts as galaxy and can open then return", "[slr][SystemView]")
{
	MainView mainView;
	REQUIRE(mainView.isGalaxy());
	REQUIRE(mainView.kind() == MainViewKind::Galaxy);
	REQUIRE_FALSE(mainView.openedSystemId().has_value());

	mainView.openSystem(HelionSystemId);
	REQUIRE(mainView.isSystem());
	REQUIRE(mainView.kind() == MainViewKind::System);
	REQUIRE(mainView.openedSystemId().has_value());
	REQUIRE(*mainView.openedSystemId() == HelionSystemId);

	mainView.returnToGalaxy();
	REQUIRE(mainView.isGalaxy());
	REQUIRE_FALSE(mainView.openedSystemId().has_value());
}

TEST_CASE("opening Helion still reads star, out radius, jump, and yard", "[slr][SystemView]")
{
	const WorldData world = makeStarterMap();
	MainView mainView;
	mainView.openSystem(HelionSystemId);
	REQUIRE(mainView.isSystem());

	const SystemLayout* layout = world.systemLayoutBySystemId(*mainView.openedSystemId());
	REQUIRE(layout != nullptr);
	REQUIRE(layout->starSize == Catch::Approx(HelionStarSize));
	REQUIRE(outRadiusForLayout(*layout) == Catch::Approx(outRadiusFromStarSize(HelionStarSize)));
	REQUIRE(starPlaceForSystem(world, HelionSystemId) != nullptr);
	REQUIRE(jumpPlaceForLaneNeighbor(world, HelionSystemId, KestrelSystemId) != nullptr);
	REQUIRE(yardPlaceForSystem(world, HelionSystemId) != nullptr);
}

TEST_CASE("opening a system with no layout does not invent places", "[slr][SystemView]")
{
	const WorldData world = makeStarterMap();
	MainView mainView;
	mainView.openSystem(KestrelSystemId);
	REQUIRE(mainView.isSystem());
	REQUIRE(*mainView.openedSystemId() == KestrelSystemId);
	REQUIRE(world.systemLayoutBySystemId(KestrelSystemId) == nullptr);
	REQUIRE(starPlaceForSystem(world, KestrelSystemId) == nullptr);
	REQUIRE(yardPlaceForSystem(world, KestrelSystemId) == nullptr);
}

TEST_CASE("system main view is not galaxy so galaxy pick can be skipped", "[slr][SystemView]")
{
	MainView mainView;
	REQUIRE(mainView.isGalaxy());
	mainView.openSystem(HelionSystemId);
	REQUIRE_FALSE(mainView.isGalaxy());
	REQUIRE(mainView.isSystem());
}

}
}
