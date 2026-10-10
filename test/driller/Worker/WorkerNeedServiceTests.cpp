#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Core/Game.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Need.hpp>
#include <Entities/Worker.hpp>
#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/GameCommandService.hpp>
#include <Services/JobCreationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/ShuttleCreationService.hpp>
#include <Services/ShuttlePrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/UpgradeService.hpp>
#include <Services/WorkerCreationService.hpp>
#include <Services/WorkerNeedService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Services/WorkerRecruitmentService.hpp>

namespace drl
{
namespace WorkerNeedServiceTests
{

	struct Fixture
	{
		WorkerData workerData;
		NeedPrototypeService needPrototypes;
		WorkerNeedService service{ workerData, needPrototypes };

		void registerNeed(const std::string& name, float decayPerSecond)
		{
			NeedPrototype prototype{};
			prototype.name = name;
			prototype.decayPerSecond = decayPerSecond;
			needPrototypes.registerPrototype(std::move(prototype));
		}

		WorkerInstance& addWorker()
		{
			WorkerInstance& worker = workerData.workers.emplace_back();
			worker.id = static_cast<WorkerId>(workerData.workers.size());
			for (const NeedId needId : needPrototypes.registeredIds())
			{
				worker.needValues[needId] = kNeedValueFull;
			}
			return worker;
		}
	};

	TEST_CASE("no workers does not throw", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		f.service.update(1.0f);
	}

	TEST_CASE("one sim-second at 1 per second decays from 100 to 99", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		WorkerInstance& worker = f.addWorker();
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 99.0f);
	}

	TEST_CASE("each need uses its own decay rate", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		f.registerNeed("Need_Food", 0.8f);
		WorkerInstance& worker = f.addWorker();
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 99.0f);
		REQUIRE_THAT(worker.needValues.at(needIdFromName("Need_Food")), Catch::Matchers::WithinAbs(99.2f, 0.0001f));
	}

	TEST_CASE("decay clamps at 0", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		WorkerInstance& worker = f.addWorker();
		worker.needValues[needIdFromName("Need_Sleep")] = 0.5f;
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 0.0f);
	}

	TEST_CASE("Game SimSpeed 2 decays twice as fast", "[drl][WorkerNeedService]")
	{
		Fixture needs;
		needs.registerNeed("Need_Sleep", 1.0f);
		WorkerInstance& worker = needs.addWorker();

		TerrainData terrainData;
		JobData jobData;
		TerrainAlterationService terrain{ terrainData };
		EconomyResourceService economy;
		JobPrototypeService jobPrototypes;
		JobCreationService jobCreation{ jobData, jobPrototypes, terrain };
		WorkerPrototypeService workerPrototypes;
		WorkerRecruitmentService recruitment{ needs.workerData, workerPrototypes };
		BuildingData buildingData;
		BuildingPrototypeService buildingPrototypes;
		WorkerCreationService workerCreation{
			needs.workerData,
			workerPrototypes,
			buildingData,
			buildingPrototypes,
			needs.needPrototypes };
		BuildingPlacementService buildings{ buildingData, terrain, recruitment, jobCreation, buildingPrototypes };
		ShuttleData shuttleData;
		ShuttlePrototypeService shuttlePrototypes;
		ShuttleCreationService shuttleCreation{ shuttleData, shuttlePrototypes };
		UpgradeData upgradeData;
		UpgradeService upgrades{ upgradeData };
		GameCommandService commands{
			terrain,
			economy,
			jobCreation,
			workerCreation,
			buildings,
			buildingPrototypes,
			shuttleCreation,
			upgrades,
			needs.workerData };

		float simSpeed = 2.0f;
		Game game(commands, simSpeed);
		game.addTickService(needs.service);
		game.update(1.0f);

		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 98.0f);
	}

}
}
