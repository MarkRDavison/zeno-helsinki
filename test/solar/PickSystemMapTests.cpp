#include <Views/SystemMapCamera.hpp>
#include <Views/SystemMapMoverLayout.hpp>
#include <World/PickSystemMap.hpp>
#include <World/StarterMap.hpp>
#include <World/SystemPlaces.hpp>
#include <World/WorldData.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

namespace slr
{
namespace PickSystemMapTests
{

constexpr long long HelionSystemId = 1;

Fleet& addParkedFleetAtHelionYard(WorldData& world, const std::string& name)
{
	world.fleets.push_back(Fleet{
		.id = nextFleetId(world),
		.empireId = HelionEmpireId,
		.name = name,
		.systemId = HelionSystemId,
		.placeId = HelionYardPlaceId,
		.motion = { .speed = 50.0f } });
	return world.fleets.back();
}

SystemMapCamera makeTestCamera(const SystemLayout& layout)
{
	SystemMapCamera camera;
	const ImVec2 canvasSize{ 800.0f, 600.0f };
	camera.setCanvas(ImVec2{ 0.0f, 100.0f }, canvasSize);
	camera.resetToFit(layout);
	return camera;
}

TEST_CASE("parked fleets at the same yard orbit to different system positions", "[slr][PickSystemMap]")
{
	WorldData world = makeStarterMap();
	const Fleet& extra = addParkedFleetAtHelionYard(world, "Yard Picket");

	const SystemLayout* layout = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(layout != nullptr);

	const std::vector<SystemMapMoverDraw> movers = listMoversForSystem(world, HelionSystemId, *layout);
	REQUIRE(movers.size() >= 2);

	glm::vec2 homePos{ 0.0f, 0.0f };
	glm::vec2 extraPos{ 0.0f, 0.0f };
	bool foundHome = false;
	bool foundExtra = false;
	for (const auto& mover : movers)
	{
		if (mover.ref.kind != SystemMapMoverKind::Fleet)
		{
			continue;
		}

		if (mover.ref.id == 1)
		{
			homePos = mover.systemPosition;
			foundHome = true;
		}
		else if (mover.ref.id == extra.id)
		{
			extraPos = mover.systemPosition;
			foundExtra = true;
		}
	}

	REQUIRE(foundHome);
	REQUIRE(foundExtra);
	REQUIRE(glm::length(homePos - extraPos) > 1.0f);
}

TEST_CASE("pickSystemMap prefers fleet on orbit over yard centre", "[slr][PickSystemMap]")
{
	WorldData world = makeStarterMap();
	const SystemLayout* layout = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(layout != nullptr);

	const SystemMapCamera camera = makeTestCamera(*layout);
	const std::vector<SystemMapMoverDraw> movers = listMoversForSystem(world, HelionSystemId, *layout);
	REQUIRE_FALSE(movers.empty());

	const SystemMapMoverDraw& home = movers.front();
	REQUIRE(home.ref.kind == SystemMapMoverKind::Fleet);
	const ImVec2 fleetScreen = camera.systemSpaceToScreen(home.systemPosition);

	const SystemMapPick onFleet = pickSystemMap(
		world,
		HelionSystemId,
		*layout,
		camera,
		movers,
		fleetScreen);
	REQUIRE(onFleet.kind == SystemMapPickKind::Fleet);
	REQUIRE(onFleet.id == 1);

	const SystemPlace* yard = yardPlaceForSystem(world, HelionSystemId);
	REQUIRE(yard != nullptr);
	const ImVec2 yardCentre = camera.systemSpaceToScreen(yard->position);
	const SystemMapPick onYard = pickSystemMap(
		world,
		HelionSystemId,
		*layout,
		camera,
		movers,
		yardCentre);
	REQUIRE(onYard.kind == SystemMapPickKind::Place);
	REQUIRE(onYard.id == HelionYardPlaceId);
}

TEST_CASE("pickSystemMap resolves two fleets on separate orbit slots", "[slr][PickSystemMap]")
{
	WorldData world = makeStarterMap();
	const Fleet& extra = addParkedFleetAtHelionYard(world, "Yard Picket");
	const SystemLayout* layout = world.systemLayoutBySystemId(HelionSystemId);
	REQUIRE(layout != nullptr);

	const SystemMapCamera camera = makeTestCamera(*layout);
	const std::vector<SystemMapMoverDraw> movers = listMoversForSystem(world, HelionSystemId, *layout);

	ImVec2 homeScreen{};
	ImVec2 extraScreen{};
	for (const auto& mover : movers)
	{
		if (mover.ref.kind != SystemMapMoverKind::Fleet)
		{
			continue;
		}

		if (mover.ref.id == 1)
		{
			homeScreen = camera.systemSpaceToScreen(mover.systemPosition);
		}
		else if (mover.ref.id == extra.id)
		{
			extraScreen = camera.systemSpaceToScreen(mover.systemPosition);
		}
	}

	const SystemMapPick homePick =
		pickSystemMap(world, HelionSystemId, *layout, camera, movers, homeScreen);
	const SystemMapPick extraPick =
		pickSystemMap(world, HelionSystemId, *layout, camera, movers, extraScreen);
	REQUIRE(homePick.kind == SystemMapPickKind::Fleet);
	REQUIRE(homePick.id == 1);
	REQUIRE(extraPick.kind == SystemMapPickKind::Fleet);
	REQUIRE(extraPick.id == extra.id);
}

}
}
