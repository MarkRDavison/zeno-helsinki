#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <Views/PlacementGhost.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace BuildingGhostViewTests
{

	TEST_CASE("placementGhostColor is green only when placeable and affordable", "[drl][BuildingGhostView]")
	{
		REQUIRE(placementGhostColor(true, true) == glm::vec4(0.35f, 1.0f, 0.35f, 0.5f));
		REQUIRE(placementGhostColor(false, true) == glm::vec4(1.0f, 0.35f, 0.35f, 0.5f));
		REQUIRE(placementGhostColor(true, false) == glm::vec4(1.0f, 0.35f, 0.35f, 0.5f));
		REQUIRE(placementGhostColor(false, false) == glm::vec4(1.0f, 0.35f, 0.35f, 0.5f));
	}

	TEST_CASE("ghost predicate uses canPlacePrototype and canAfford", "[drl][BuildingGhostView]")
	{
		TerrainData terrainData;
		JobData jobData;
		WorkerData workerData;
		BuildingData buildingData;
		TerrainAlterationService terrain{ terrainData };
		JobPrototypeService jobPrototypes;
		JobCreationService jobs{ jobData, jobPrototypes, terrain };
		WorkerPrototypeService workerPrototypes;
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingPrototypeService buildingPrototypes;
		BuildingPlacementService placement{ buildingData, terrain, recruitment, jobs, buildingPrototypes };
		EconomyResourceService economy;

		BuildingPrototype bunk{};
		bunk.name = "Building_Bunk";
		bunk.cost = 50;
		bunk.size = glm::ivec2(2, 1);
		buildingPrototypes.registerPrototype(std::move(bunk));
		const auto bunkId = prototypeIdFromName("Building_Bunk");

		REQUIRE(terrain.digShaft(0));
		REQUIRE(terrain.digTile(0, 1));
		REQUIRE(terrain.digTile(0, 2));
		economy.set(ResourceMoney, 500);

		const bool dugPlace = placement.canPlacePrototype(bunkId, 0, 1);
		const bool dugAfford = economy.canAfford(ResourceMoney, 50);
		REQUIRE(dugPlace);
		REQUIRE(dugAfford);
		REQUIRE(placementGhostColor(dugPlace, dugAfford) == glm::vec4(0.35f, 1.0f, 0.35f, 0.5f));

		const bool undugPlace = placement.canPlacePrototype(bunkId, 0, 3);
		REQUIRE_FALSE(undugPlace);
		REQUIRE(placementGhostColor(undugPlace, dugAfford) == glm::vec4(1.0f, 0.35f, 0.35f, 0.5f));

		economy.set(ResourceMoney, 0);
		const bool brokeAfford = economy.canAfford(ResourceMoney, 50);
		REQUIRE_FALSE(brokeAfford);
		REQUIRE(placementGhostColor(dugPlace, brokeAfford) == glm::vec4(1.0f, 0.35f, 0.35f, 0.5f));
	}

}
}
