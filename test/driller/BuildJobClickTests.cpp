#include <catch2/catch_test_macros.hpp>
#include <Core/BuildJobClick.hpp>
#include <Core/GameCommand.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
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
		WorkerRecruitmentService recruitment{ workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		NeedPrototypeService needPrototypes;
		WorkerCreationService workerCreation{ workerData, workerPrototypes, buildingData, buildingPrototypes, needPrototypes };
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		ShuttleData shuttleData;
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		UpgradeData upgradeData;
		UpgradeService upgrades{ upgradeData };
		GameCommandService commands{ terrain, economy, jobCreation, workerCreation, buildings, buildingPrototypes, shuttleCreation, upgrades, workerData };

		Fixture()
		{
			JobPrototype prototype{};
			prototype.name = "Job_Build_Building";
			prototype.work = 5.0f;
			jobPrototypes.registerPrototype(std::move(prototype));
			BuildingPrototype bunk{};
			bunk.name = "Building_Bunk";
			bunk.cost = 50;
			buildingPrototypes.registerPrototype(std::move(bunk));
			economy.set(ResourceMoney, 500);
			REQUIRE(terrain.digShaft(0));
		}

		void enqueueBunk(int level, int column)
		{
			enqueueBuildJob(commands, terrain, level, column, "Building_Bunk");
		}
	};

	TEST_CASE("enqueueBuildJob reserves Job_Build_Building left and right", "[drl][BuildJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE(f.terrain.digTile(0, column));
			f.enqueueBunk(0, column);
			REQUIRE(f.jobData.jobs.size() == 1);
			REQUIRE(f.jobData.jobs[0].prototypeId == jobPrototypeIdFromName("Job_Build_Building"));
			REQUIRE(f.jobData.jobs[0].additionalPrototypeId == prototypeIdFromName("Building_Bunk"));
			REQUIRE(f.jobData.jobs[0].tile == glm::ivec2(column, 0));
			REQUIRE(f.terrain.getTile(0, column).jobReserved);
			REQUIRE(f.economy.get(ResourceMoney) == 450);
		}
	}

	TEST_CASE("enqueueBuildJob initialiseTile then reserves missing tile left and right", "[drl][BuildJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE_FALSE(f.terrain.doesTileExist(0, column));
			f.enqueueBunk(0, column);
			REQUIRE(f.terrain.doesTileExist(0, column));
			REQUIRE(f.jobData.jobs.size() == 1);
			REQUIRE(f.terrain.getTile(0, column).jobReserved);
			REQUIRE_FALSE(f.terrain.isTileDugOut(0, column));
			REQUIRE(f.economy.get(ResourceMoney) == 450);
		}
	}

	TEST_CASE("enqueueBuildJob refuses when the player cannot afford", "[drl][BuildJobClick]")
	{
		for (const int column : { 1, -1 })
		{
			Fixture f;
			REQUIRE(f.terrain.digTile(0, column));
			f.economy.set(ResourceMoney, 49);
			f.enqueueBunk(0, column);
			REQUIRE(f.jobData.jobs.empty());
			REQUIRE(f.economy.get(ResourceMoney) == 49);
			REQUIRE_FALSE(f.terrain.getTile(0, column).jobReserved);
		}
	}

	TEST_CASE("enqueueBuildJob with zero cost still creates a job", "[drl][BuildJobClick]")
	{
		Fixture f;
		BuildingPrototype freeHut{};
		freeHut.name = "Building_Free";
		freeHut.cost = 0;
		f.buildingPrototypes.registerPrototype(std::move(freeHut));
		REQUIRE(f.terrain.digTile(0, 1));
		enqueueBuildJob(f.commands, f.terrain, 0, 1, "Building_Free");
		REQUIRE(f.jobData.jobs.size() == 1);
		REQUIRE(f.economy.get(ResourceMoney) == 500);
	}

	TEST_CASE("enqueueBuildJob unknown prototype does not pay", "[drl][BuildJobClick]")
	{
		Fixture f;
		REQUIRE(f.terrain.digTile(0, 1));
		enqueueBuildJob(f.commands, f.terrain, 0, 1, "Building_Missing");
		REQUIRE(f.jobData.jobs.empty());
		REQUIRE(f.economy.get(ResourceMoney) == 500);
		REQUIRE_FALSE(f.terrain.getTile(0, 1).jobReserved);
	}

}
}
