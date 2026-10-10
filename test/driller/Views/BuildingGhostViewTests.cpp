#include <catch2/catch_test_macros.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <Views/PlacementGhost.hpp>
#include <helsinki/System/glm.hpp>
#include <vector>

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

	TEST_CASE("queuedBuildGhostColor is amber 0.5", "[drl][BuildingGhostView]")
	{
		REQUIRE(queuedBuildGhostColor() == glm::vec4(1.0f, 0.78f, 0.28f, 0.5f));
	}

	TEST_CASE("queued ghosts are Job_Build_Building with a registered additional prototype", "[drl][BuildingGhostView]")
	{
		JobData jobData;
		BuildingPrototypeService buildingPrototypes;
		BuildingPrototype bunk{};
		bunk.name = "Building_Bunk";
		buildingPrototypes.registerPrototype(std::move(bunk));

		JobInstance build{};
		build.id = 1;
		build.prototypeId = jobPrototypeIdFromName("Job_Build_Building");
		build.additionalPrototypeId = prototypeIdFromName("Building_Bunk");
		jobData.jobs.push_back(build);

		JobInstance dig{};
		dig.id = 2;
		dig.prototypeId = jobPrototypeIdFromName("Job_Dig");
		jobData.jobs.push_back(dig);

		JobInstance mine{};
		mine.id = 3;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		mine.additionalPrototypeId = prototypeIdFromName("Building_Bunk");
		jobData.jobs.push_back(mine);

		JobInstance unknownBuilding{};
		unknownBuilding.id = 4;
		unknownBuilding.prototypeId = jobPrototypeIdFromName("Job_Build_Building");
		unknownBuilding.additionalPrototypeId = prototypeIdFromName("Building_Missing");
		jobData.jobs.push_back(unknownBuilding);

		REQUIRE(queuedBuildGhostJobIds(jobData, buildingPrototypes) == std::vector<JobId>{ 1 });
	}

	TEST_CASE("CancelJob leaves no queued build ghost for that job", "[drl][BuildingGhostView]")
	{
		TerrainData terrainData;
		JobData jobData;
		WorkerData workerData;
		BuildingData buildingData;
		ShuttleData shuttleData;
		UpgradeData upgradeData;
		TerrainAlterationService terrain{ terrainData };
		EconomyResourceService economy;
		economy.setMax(ResourceMoney, -1);
		economy.set(ResourceMoney, 500);
		JobPrototypeService jobPrototypes;
		JobCreationService jobs{ jobData, jobPrototypes, terrain };
		WorkerPrototypeService workerPrototypes;
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingPrototypeService buildingPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes, buildingData, buildingPrototypes };
		BuildingPlacementService placement{ buildingData, terrain, recruitment, jobs, buildingPrototypes };
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttles{ shuttleData, shuttlePrototypes };
		UpgradeService upgrades{ upgradeData };
		GameCommandService commands{
			terrain,
			economy,
			jobs,
			workerCreation,
			placement,
			buildingPrototypes,
			shuttles,
			upgrades,
			workerData };

		JobPrototype buildJob{};
		buildJob.name = kJobBuildBuilding;
		buildJob.work = 5.0f;
		jobPrototypes.registerPrototype(std::move(buildJob));
		BuildingPrototype bunk{};
		bunk.name = "Building_Bunk";
		bunk.cost = 50;
		buildingPrototypes.registerPrototype(std::move(bunk));

		REQUIRE(terrain.digShaft(0));
		terrain.initialiseTile(0, 1);
		REQUIRE(commands.execute(GameCommand::createJob(
			kJobBuildBuilding,
			"Building_Bunk",
			0,
			1,
			CommandSource::Player,
			CommandContext::PlacingBuilding)));
		REQUIRE(queuedBuildGhostJobIds(jobData, buildingPrototypes).size() == 1);

		REQUIRE(commands.execute(GameCommand::cancelJob(
			0,
			1,
			CommandSource::Player,
			CommandContext::CancellingJob)));
		REQUIRE(queuedBuildGhostJobIds(jobData, buildingPrototypes).empty());
	}

}
}
