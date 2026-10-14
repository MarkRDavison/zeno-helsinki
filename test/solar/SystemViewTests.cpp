#include <Views/MainView.hpp>
#include <Views/SystemView.hpp>
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
	WorldData world;
	world.systems.push_back(StarSystem{ .id = 99, .name = "Empty" });
	MainView mainView;
	mainView.openSystem(99);
	REQUIRE(mainView.isSystem());
	REQUIRE(*mainView.openedSystemId() == 99);
	REQUIRE(world.systemLayoutBySystemId(99) == nullptr);
	REQUIRE(starPlaceForSystem(world, 99) == nullptr);
	REQUIRE(yardPlaceForSystem(world, 99) == nullptr);
}

TEST_CASE("system main view is not galaxy so galaxy pick can be skipped", "[slr][SystemView]")
{
	MainView mainView;
	REQUIRE(mainView.isGalaxy());
	mainView.openSystem(HelionSystemId);
	REQUIRE_FALSE(mainView.isGalaxy());
	REQUIRE(mainView.isSystem());
}

TEST_CASE("pickSystemPlace hits Helion star, yard, and Kestrel jump", "[slr][SystemView]")
{
	const WorldData world = makeStarterMap();
	const SystemLayout* helion = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(helion != nullptr);

	const SystemPlace* star = pickSystemPlace(*helion, starPositionInSystemSpace(*helion));
	REQUIRE(star != nullptr);
	REQUIRE(star->kind == SystemPlaceKind::Star);
	REQUIRE(star->id == HelionStarPlaceId);

	const SystemPlace* yard = pickSystemPlace(*helion, HelionYardPlaceOffset);
	REQUIRE(yard != nullptr);
	REQUIRE(yard->kind == SystemPlaceKind::Yard);
	REQUIRE(yard->id == HelionYardPlaceId);

	const SystemPlace* jump = jumpPlaceForLaneNeighbor(*helion, KestrelSystemId);
	REQUIRE(jump != nullptr);
	const SystemPlace* pickedJump = pickSystemPlace(*helion, jump->position);
	REQUIRE(pickedJump != nullptr);
	REQUIRE(pickedJump->kind == SystemPlaceKind::Jump);
	REQUIRE(pickedJump->id == HelionKestrelJumpPlaceId);

	REQUIRE(pickSystemPlace(*helion, glm::vec2{ 400.0f, 400.0f }) == nullptr);
}

TEST_CASE("pick at Helion yard is still the yard when Home Fleet is parked there", "[slr][SystemView]")
{
	const WorldData world = makeStarterMap();
	const SystemLayout* helion = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(helion != nullptr);
	const Fleet* home = world.fleetById(1);
	REQUIRE(home != nullptr);
	REQUIRE(home->placeId == HelionYardPlaceId);

	const SystemPlace* yard = pickSystemPlace(*helion, HelionYardPlaceOffset);
	REQUIRE(yard != nullptr);
	REQUIRE(yard->kind == SystemPlaceKind::Yard);
	REQUIRE(yard->id == HelionYardPlaceId);
}

TEST_CASE("pickSystemPlace prefers yard over star when both would hit", "[slr][SystemView]")
{
	SystemLayout layout;
	layout.starSize = 24.0f;
	layout.places.push_back(SystemPlace{
		.id = 1,
		.kind = SystemPlaceKind::Star,
		.position = { 0.0f, 0.0f } });
	layout.places.push_back(SystemPlace{
		.id = 2,
		.kind = SystemPlaceKind::Yard,
		.position = { 10.0f, 0.0f } });
	const SystemPlace* picked = pickSystemPlace(layout, glm::vec2{ 10.0f, 0.0f });
	REQUIRE(picked != nullptr);
	REQUIRE(picked->kind == SystemPlaceKind::Yard);
	REQUIRE(placeById(layout, 2) == picked);
}

TEST_CASE("leaving the system view clears selected place", "[slr][SystemView]")
{
	SystemView view;
	MainView mainView;
	mainView.openSystem(HelionSystemId);
	view.setSelectedPlaceId(HelionYardPlaceId);
	REQUIRE(view.selectedPlaceId().has_value());

	mainView.returnToGalaxy();
	view.applyMainView(mainView);
	REQUIRE_FALSE(view.selectedPlaceId().has_value());
}

}
}
