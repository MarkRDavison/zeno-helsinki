#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Core/Game.hpp>
#include <Entities/Data/BuildingData.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/UpgradeData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
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
#include <optional>

namespace drl
{
namespace WorkerNeedServiceTests
{

	struct Fixture
	{
		WorkerData workerData;
		NeedPrototypeService needPrototypes;
		JobData jobData;
		JobPrototypeService jobPrototypes;
		WorkerNeedService service{ workerData, needPrototypes, jobData, jobPrototypes };

		void registerNeed(
			const std::string& name,
			float decayPerSecond,
			float seekBelow = 0.0f,
			float collapseBelow = -1.0f)
		{
			NeedPrototype prototype{};
			prototype.name = name;
			prototype.decayPerSecond = decayPerSecond;
			prototype.seekBelow = seekBelow;
			prototype.collapseBelow = collapseBelow;
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

	TEST_CASE("idle worker uses base decay not job modifiers", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		job.needDecay[needIdFromName("Need_Sleep")] = NeedDecayModifier{ 2.0f, 2.0f };
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		worker.state = WorkerState::Idle;
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 99.0f);
	}

	TEST_CASE("moving worker uses base decay", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		job.needDecay[needIdFromName("Need_Sleep")] = NeedDecayModifier{ 2.0f, 2.0f };
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		worker.state = WorkerState::MovingToJob;
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 99.0f);
	}

	TEST_CASE("working job applies multiplier and additive", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		job.needDecay[needIdFromName("Need_Sleep")] = NeedDecayModifier{ 2.0f, 2.0f };
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		worker.state = WorkerState::WorkingJob;
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 96.0f);
	}

	TEST_CASE("working job missing needDecay key uses base rate", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f);
		f.registerNeed("Need_Food", 1.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		job.needDecay[needIdFromName("Need_Sleep")] = NeedDecayModifier{ 2.0f, 0.0f };
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		worker.state = WorkerState::WorkingJob;
		f.service.update(1.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 98.0f);
		REQUIRE(worker.needValues.at(needIdFromName("Need_Food")) == 99.0f);
	}

	TEST_CASE("sleep at 24 is seek", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f, 25.0f, 0.0f);
		WorkerInstance& worker = f.addWorker();
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;
		REQUIRE(f.service.classify(worker, needIdFromName("Need_Sleep")) == NeedBand::Seek);
	}

	TEST_CASE("sleep at seekBelow is ok", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f, 25.0f, 0.0f);
		WorkerInstance& worker = f.addWorker();
		worker.needValues[needIdFromName("Need_Sleep")] = 25.0f;
		REQUIRE(f.service.classify(worker, needIdFromName("Need_Sleep")) == NeedBand::Ok);
	}

	TEST_CASE("sleep at 0 with collapseBelow 0 is collapse", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 1.0f, 25.0f, 0.0f);
		WorkerInstance& worker = f.addWorker();
		worker.needValues[needIdFromName("Need_Sleep")] = 0.0f;
		REQUIRE(f.service.classify(worker, needIdFromName("Need_Sleep")) == NeedBand::Collapse);
	}

	TEST_CASE("recreation without collapse never collapses", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Recreation", 0.4f, 10.0f, -1.0f);
		WorkerInstance& worker = f.addWorker();
		worker.needValues[needIdFromName("Need_Recreation")] = 0.0f;
		REQUIRE(f.service.classify(worker, needIdFromName("Need_Recreation")) == NeedBand::Seek);
		worker.needValues[needIdFromName("Need_Recreation")] = 10.0f;
		REQUIRE(f.service.classify(worker, needIdFromName("Need_Recreation")) == NeedBand::Ok);
	}

	TEST_CASE("seeking worker unassigns repeating work job", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		mine.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		f.service.update(0.0f);

		REQUIRE(worker.allocatedJobId == 0);
		REQUIRE(worker.state == WorkerState::Idle);
		REQUIRE(mine.allocatedWorkerId == 0);
		REQUIRE(f.jobData.jobs.size() == 1);
		REQUIRE_FALSE(mine.requiresRemoval);
	}

	TEST_CASE("seeking worker keeps restore job for chosen need", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Sleep";
		job.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& sleep = f.jobData.jobs.emplace_back();
		sleep.id = 1;
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = sleep.id;
		sleep.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		f.service.update(0.0f);

		REQUIRE(worker.allocatedJobId == sleep.id);
		REQUIRE(sleep.allocatedWorkerId == worker.id);
		REQUIRE(worker.state == WorkerState::WorkingJob);
	}

	TEST_CASE("collapsed worker unassigns current job", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Mine";
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& mine = f.jobData.jobs.emplace_back();
		mine.id = 1;
		mine.prototypeId = jobPrototypeIdFromName("Job_Mine");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = mine.id;
		mine.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 0.0f;

		f.service.update(0.0f);

		REQUIRE(worker.allocatedJobId == 0);
		REQUIRE(mine.allocatedWorkerId == 0);
		REQUIRE(f.service.chosenSeekNeed(worker) == std::nullopt);
		REQUIRE(f.service.hasCollapsedNeed(worker));
	}

	TEST_CASE("working restore job raises the need", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Sleep";
		job.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& sleep = f.jobData.jobs.emplace_back();
		sleep.id = 1;
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = sleep.id;
		sleep.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		f.service.update(1.0f);

		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 39.0f);
		REQUIRE(worker.allocatedJobId == sleep.id);
		REQUIRE(worker.state == WorkerState::WorkingJob);
	}

	TEST_CASE("full restore unassigns worker and leaves repeating job", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Sleep";
		job.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& sleep = f.jobData.jobs.emplace_back();
		sleep.id = 1;
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = sleep.id;
		sleep.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 90.0f;

		f.service.update(1.0f);

		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == kNeedValueFull);
		REQUIRE(worker.allocatedJobId == 0);
		REQUIRE(worker.state == WorkerState::Idle);
		REQUIRE(sleep.allocatedWorkerId == 0);
		REQUIRE(f.jobData.jobs.size() == 1);
		REQUIRE_FALSE(sleep.requiresRemoval);
	}

	TEST_CASE("restoreUntil below 100 unassigns when reached", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Sleep";
		job.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		job.restoreUntil = 50.0f;
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& sleep = f.jobData.jobs.emplace_back();
		sleep.id = 1;
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = sleep.id;
		sleep.allocatedWorkerId = worker.id;
		worker.state = WorkerState::WorkingJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 40.0f;

		f.service.update(1.0f);

		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 55.0f);
		REQUIRE(worker.state == WorkerState::Idle);
		REQUIRE(sleep.allocatedWorkerId == 0);
	}

	TEST_CASE("moving to restore job does not restore", "[drl][WorkerNeedService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 0.0f, 25.0f, 0.0f);
		JobPrototype job{};
		job.name = "Job_Sleep";
		job.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.jobPrototypes.registerPrototype(std::move(job));
		JobInstance& sleep = f.jobData.jobs.emplace_back();
		sleep.id = 1;
		sleep.prototypeId = jobPrototypeIdFromName("Job_Sleep");
		WorkerInstance& worker = f.addWorker();
		worker.allocatedJobId = sleep.id;
		sleep.allocatedWorkerId = worker.id;
		worker.state = WorkerState::MovingToJob;
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		f.service.update(1.0f);

		REQUIRE(worker.needValues.at(needIdFromName("Need_Sleep")) == 24.0f);
		REQUIRE(worker.allocatedJobId == sleep.id);
	}

}
}
