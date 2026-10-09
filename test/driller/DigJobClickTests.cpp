#include <catch2/catch_test_macros.hpp>
#include <Core/DigJobClick.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace DigJobClickTests
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
		ShuttleData shuttleData;
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings, shuttleCreation };

		Fixture()
		{
			JobPrototype prototype{};
			prototype.name = "Job_Dig";
			prototype.work = 2.0f;
			jobPrototypes.registerPrototype(std::move(prototype));
			REQUIRE(terrain.digShaft(0));
		}
	};

	int jobCountAt(const JobData& jobData, int column)
	{
		int count = 0;
		for (const JobInstance& job : jobData.jobs)
		{
			if (job.tile.x == column && job.prototypeId == jobPrototypeIdFromName("Job_Dig"))
			{
				++count;
			}
		}
		return count;
	}

	TEST_CASE("single click on undug next tile enqueues Job_Dig left and right", "[drl][DigJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			enqueueDigJobs(f.commands, f.terrain, 0, column, false);
			REQUIRE(jobCountAt(f.jobData, column) == 1);
			REQUIRE(f.terrain.getTile(0, column).jobReserved);
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, column));
			REQUIRE(f.jobData.jobs.size() == 1);
		}
	}

	TEST_CASE("click on a dug tile enqueues no job left and right", "[drl][DigJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE(f.terrain.digTile(0, column));
			enqueueDigJobs(f.commands, f.terrain, 0, column, false);
			REQUIRE(f.jobData.jobs.empty());
			REQUIRE_FALSE(f.terrain.getTile(0, column).jobReserved);
		}
	}

	TEST_CASE("shift-range enqueues only undug cells left and right", "[drl][DigJobClick]")
	{
		for (const int sign : { 1, -1 })
		{
			Fixture f;
			REQUIRE(f.terrain.digTile(0, sign));
			enqueueDigJobs(f.commands, f.terrain, 0, 3 * sign, true);
			REQUIRE(jobCountAt(f.jobData, sign) == 0);
			REQUIRE(jobCountAt(f.jobData, 2 * sign) == 1);
			REQUIRE(jobCountAt(f.jobData, 3 * sign) == 1);
			REQUIRE(f.jobData.jobs.size() == 2);
			REQUIRE(f.terrain.getTile(0, 2 * sign).jobReserved);
			REQUIRE(f.terrain.getTile(0, 3 * sign).jobReserved);
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, 2 * sign));
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, 3 * sign));
		}
	}

	TEST_CASE("shift-range initialiseTile expands past cavern left and right", "[drl][DigJobClick]")
	{
		for (const int sign : { 1, -1 })
		{
			Fixture f;
			REQUIRE_FALSE(f.terrain.doesTileExist(0, sign));
			REQUIRE_FALSE(f.terrain.doesTileExist(0, 2 * sign));
			enqueueDigJobs(f.commands, f.terrain, 0, 2 * sign, true);
			REQUIRE(f.terrain.doesTileExist(0, sign));
			REQUIRE(f.terrain.doesTileExist(0, 2 * sign));
			REQUIRE(jobCountAt(f.jobData, sign) == 1);
			REQUIRE(jobCountAt(f.jobData, 2 * sign) == 1);
			REQUIRE(f.terrain.getTile(0, 2 * sign).jobReserved);
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, 2 * sign));
		}
	}

}
}
