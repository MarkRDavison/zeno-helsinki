#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Need.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace BuildingPlacementServiceTests
{

	struct Fixture
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

		void registerBunk()
		{
			BuildingPrototype prototype{};
			prototype.name = "Building_Bunk";
			prototype.size = glm::ivec2(2, 1);
			buildingPrototypes.registerPrototype(std::move(prototype));
		}

		void registerHut()
		{
			WorkerPrototype worker{};
			worker.name = "Worker_Builder";
			workerPrototypes.registerPrototype(std::move(worker));

			BuildingPrototype prototype{};
			prototype.name = "Building_Builders_Hut";
			prototype.size = glm::ivec2(2, 1);
			prototype.requiredWorkers["Worker_Builder"] = 2;
			buildingPrototypes.registerPrototype(std::move(prototype));
		}

		void registerMine()
		{
			JobPrototype job{};
			job.name = "Job_Mine";
			job.work = 4.0f;
			job.repeats = true;
			jobPrototypes.registerPrototype(std::move(job));

			BuildingPrototype prototype{};
			prototype.name = "Building_Mine";
			prototype.size = glm::ivec2(3, 1);
			prototype.providedJobs.emplace_back("Job_Mine", glm::vec2(1.0f, 0.0f));
			buildingPrototypes.registerPrototype(std::move(prototype));
		}

		void digFootprint(int level, int startColumn, int width)
		{
			REQUIRE(terrain.digShaft(level));
			if (startColumn > 0)
			{
				for (int x = 1; x <= startColumn + width - 1; ++x)
				{
					REQUIRE(terrain.digTile(level, x));
				}
			}
			else
			{
				for (int x = -1; x >= startColumn; --x)
				{
					REQUIRE(terrain.digTile(level, x));
				}
			}
		}
	};

	TEST_CASE("unknown prototype cannot place", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		REQUIRE(f.terrain.digShaft(0));
		REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
		REQUIRE_FALSE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
		REQUIRE(f.buildingData.buildings.empty());
	}

	TEST_CASE("missing tiles cannot place left and right", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		f.registerBunk();
		REQUIRE(f.terrain.digShaft(0));
		REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
		REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, -2));
		REQUIRE_FALSE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
	}

	TEST_CASE("undug tiles cannot place left and right", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		f.registerBunk();
		REQUIRE(f.terrain.digShaft(0));
		f.terrain.initialiseTile(0, 1);
		f.terrain.initialiseTile(0, 2);
		f.terrain.initialiseTile(0, -1);
		f.terrain.initialiseTile(0, -2);
		REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
		REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, -2));
	}

	TEST_CASE("job reserved tiles cannot place left and right", "[drl][BuildingPlacementService]")
	{
		for (const int startColumn : { 1, -2 })
		{
			Fixture f;
			f.registerBunk();
			f.digFootprint(0, startColumn, 2);
			f.terrain.getTile(0, startColumn).jobReserved = true;
			REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
			REQUIRE_FALSE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
		}
	}

	TEST_CASE("existing building cannot place left and right", "[drl][BuildingPlacementService]")
	{
		for (const int startColumn : { 1, -2 })
		{
			Fixture f;
			f.registerBunk();
			f.digFootprint(0, startColumn, 2);
			REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
			REQUIRE_FALSE(f.placement.canPlacePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
			REQUIRE_FALSE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
		}
	}

	TEST_CASE("valid place marks footprint and coords left and right", "[drl][BuildingPlacementService]")
	{
		for (const int startColumn : { 1, -2 })
		{
			Fixture f;
			f.registerBunk();
			f.digFootprint(0, startColumn, 2);
			REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, startColumn));
			REQUIRE(f.buildingData.buildings.size() == 1);
			REQUIRE(f.buildingData.buildings[0].coordinates == glm::ivec2(startColumn, 0));
			REQUIRE(f.terrain.getTile(0, startColumn).hasBuilding);
			if (startColumn > 0)
			{
				REQUIRE(f.terrain.getTile(0, startColumn + 1).hasBuilding);
			}
			else
			{
				REQUIRE(f.terrain.getTile(0, startColumn + 1).hasBuilding);
			}
		}
	}

	TEST_CASE("builders hut registers worker demand", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		f.registerHut();
		f.digFootprint(0, 1, 2);
		REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Builders_Hut"), 0, 1));
		REQUIRE(f.recruitment.getRequiredWorkerCount("Worker_Builder") == 2);
		REQUIRE(f.recruitment.getRequiredWorkerTypes().contains(prototypeIdFromName("Worker_Builder")));
	}

	TEST_CASE("bunk restoreSlots creates four Job_Sleep copies", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		JobPrototype sleep{};
		sleep.name = "Job_Sleep";
		sleep.work = 1.0f;
		sleep.repeats = true;
		sleep.everyoneCanPerform = true;
		sleep.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(sleep));

		BuildingPrototype prototype{};
		prototype.name = "Building_Bunk";
		prototype.size = glm::ivec2(2, 1);
		prototype.providedJobs.emplace_back("Job_Sleep", glm::vec2(0.5f, 0.0f));
		prototype.metadata[kBuildingMetadataWorkerCapacity] = 4;
		prototype.metadata[kBuildingMetadataRestoreSlots] = 4;
		f.buildingPrototypes.registerPrototype(std::move(prototype));

		f.digFootprint(0, 1, 2);
		REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Bunk"), 0, 1));
		REQUIRE(f.jobData.jobs.size() == 4);
		for (const JobInstance& job : f.jobData.jobs)
		{
			REQUIRE(job.prototypeId == jobPrototypeIdFromName("Job_Sleep"));
			REQUIRE(job.tile == glm::ivec2(1, 0));
			REQUIRE(job.offset == glm::vec2(0.5f, 0.0f));
		}
	}

	TEST_CASE("restoreSlots does not duplicate Job_Mine", "[drl][BuildingPlacementService]")
	{
		Fixture f;
		JobPrototype sleep{};
		sleep.name = "Job_Sleep";
		sleep.work = 1.0f;
		sleep.repeats = true;
		sleep.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(sleep));

		JobPrototype mine{};
		mine.name = "Job_Mine";
		mine.work = 4.0f;
		mine.repeats = true;
		f.jobPrototypes.registerPrototype(std::move(mine));

		BuildingPrototype prototype{};
		prototype.name = "Building_Mixed";
		prototype.size = glm::ivec2(3, 1);
		prototype.providedJobs.emplace_back("Job_Sleep", glm::vec2(0.5f, 0.0f));
		prototype.providedJobs.emplace_back("Job_Mine", glm::vec2(1.0f, 0.0f));
		prototype.metadata[kBuildingMetadataRestoreSlots] = 4;
		f.buildingPrototypes.registerPrototype(std::move(prototype));

		f.digFootprint(0, 1, 3);
		REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Mixed"), 0, 1));
		REQUIRE(f.jobData.jobs.size() == 5);
		int sleepJobs = 0;
		int mineJobs = 0;
		for (const JobInstance& job : f.jobData.jobs)
		{
			if (job.prototypeId == jobPrototypeIdFromName("Job_Sleep"))
			{
				++sleepJobs;
			}
			if (job.prototypeId == jobPrototypeIdFromName("Job_Mine"))
			{
				++mineJobs;
			}
		}
		REQUIRE(sleepJobs == 4);
		REQUIRE(mineJobs == 1);
	}

	TEST_CASE("mine creates provided Job_Mine without reserving", "[drl][BuildingPlacementService]")
	{
		for (const int startColumn : { 1, -3 })
		{
			Fixture f;
			f.registerMine();
			f.digFootprint(0, startColumn, 3);
			REQUIRE(f.placement.placePrototype(prototypeIdFromName("Building_Mine"), 0, startColumn));
			REQUIRE(f.jobData.jobs.size() == 1);
			REQUIRE(f.jobData.jobs[0].prototypeId == jobPrototypeIdFromName("Job_Mine"));
			REQUIRE(f.jobData.jobs[0].tile == glm::ivec2(startColumn, 0));
			REQUIRE(f.jobData.jobs[0].offset == glm::vec2(1.0f, 0.0f));
			REQUIRE_FALSE(f.terrain.getTile(0, startColumn).jobReserved);
		}
	}

}
}
