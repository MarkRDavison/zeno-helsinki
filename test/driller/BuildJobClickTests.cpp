#include <catch2/catch_test_macros.hpp>
#include <Core/BuildJobClick.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace BuildJobClickTests
{

	struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		TerrainAlterationService terrain{ terrainData };
		EconomyResourceService economy;
		JobPrototypeService jobPrototypes;
		JobCreationService jobCreation{ jobData, jobPrototypes, terrain };
		WorkerData workerData;
		WorkerPrototypeService workerPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes };
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings };

		Fixture()
		{
			JobPrototype prototype{};
			prototype.name = "Job_Build_Building";
			prototype.work = 5.0f;
			jobPrototypes.registerPrototype(std::move(prototype));
			REQUIRE(terrain.digShaft(0));
		}
	};

	TEST_CASE("enqueueBuildJob reserves Job_Build_Building left and right", "[drl][BuildJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE(f.terrain.digTile(0, column));
			enqueueBuildJob(f.commands, f.terrain, 0, column, "Building_Bunk");
			REQUIRE(f.jobData.jobs.size() == 1);
			REQUIRE(f.jobData.jobs[0].prototypeId == jobPrototypeIdFromName("Job_Build_Building"));
			REQUIRE(f.jobData.jobs[0].additionalPrototypeId == prototypeIdFromName("Building_Bunk"));
			REQUIRE(f.jobData.jobs[0].tile == glm::ivec2(column, 0));
			REQUIRE(f.terrain.getTile(0, column).jobReserved);
		}
	}

	TEST_CASE("enqueueBuildJob initialiseTile then reserves missing tile left and right", "[drl][BuildJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE_FALSE(f.terrain.doesTileExist(0, column));
			enqueueBuildJob(f.commands, f.terrain, 0, column, "Building_Bunk");
			REQUIRE(f.terrain.doesTileExist(0, column));
			REQUIRE(f.jobData.jobs.size() == 1);
			REQUIRE(f.terrain.getTile(0, column).jobReserved);
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, column));
		}
	}

}
}
