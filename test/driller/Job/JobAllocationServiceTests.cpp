#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Need.hpp>
#include <Entities/Worker.hpp>
#include <Services/JobAllocationService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerNeedService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace JobAllocationServiceTests
{

	struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		WorkerData workerData;
		TerrainAlterationService terrain{ terrainData };
		WorkerPrototypeService workerPrototypes;
		JobPrototypeService jobPrototypes;
		NeedPrototypeService needPrototypes;
		WorkerNeedService needs{ workerData, needPrototypes, jobData, jobPrototypes };
		JobAllocationService service{ jobData, workerData, terrain, workerPrototypes, jobPrototypes, needs };

		Fixture()
		{
			jobData.jobs.reserve(8);
			workerData.workers.reserve(8);
		}

		void registerWorker(const std::string& name, std::unordered_set<std::string> jobs)
		{
			WorkerPrototype prototype{};
			prototype.name = name;
			prototype.validJobPrototypes = std::move(jobs);
			workerPrototypes.registerPrototype(std::move(prototype));
		}

		void registerNeed(const std::string& name, float seekBelow, float collapseBelow, int priority)
		{
			NeedPrototype prototype{};
			prototype.name = name;
			prototype.seekBelow = seekBelow;
			prototype.collapseBelow = collapseBelow;
			prototype.priority = priority;
			needPrototypes.registerPrototype(std::move(prototype));
		}

		void registerJob(JobPrototype prototype)
		{
			jobPrototypes.registerPrototype(std::move(prototype));
		}

		JobInstance& addJob(long long id, const std::string& prototypeName, glm::ivec2 tile)
		{
			JobInstance& job = jobData.jobs.emplace_back();
			job.id = id;
			job.prototypeId = jobPrototypeIdFromName(prototypeName);
			job.tile = tile;
			return job;
		}

		WorkerInstance& addWorker(long long id, const std::string& prototypeName)
		{
			WorkerInstance& worker = workerData.workers.emplace_back();
			worker.id = id;
			worker.prototypeId = prototypeIdFromName(prototypeName);
			for (const NeedId needId : needPrototypes.registeredIds())
			{
				worker.needValues[needId] = kNeedValueFull;
			}
			return worker;
		}

		void makeTileReachable(int level, int column)
		{
			REQUIRE(terrain.digShaft(level));
			if (column != 0)
			{
				REQUIRE(terrain.digTile(level, column));
			}
		}
	};

	TEST_CASE("allocate jobs does nothing with no entities", "[drl][JobAllocationService]")
	{
		Fixture f;
		REQUIRE_NOTHROW(f.service.allocateJobs());
	}

	TEST_CASE("canWorkerPerformJob missing prototype fails", "[drl][JobAllocationService]")
	{
		Fixture f;
		JobInstance& job = f.addJob(1, "Job_Dig", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Builder");

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("canWorkerPerformJob empty valid jobs fails", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", {});
		JobInstance& job = f.addJob(1, "Job_Dig", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Builder");

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("canWorkerPerformJob name mismatch fails", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", { "Job_Mine" });
		JobInstance& job = f.addJob(1, "Job_Dig", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Builder");

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("canWorkerPerformJob unreachable tile fails", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", { "Job_Dig" });
		JobInstance& job = f.addJob(1, "Job_Dig", glm::ivec2(3, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Builder");

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("canWorkerPerformJob matching job on reachable tile succeeds", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", { "Job_Dig" });
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(1, "Job_Dig", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Builder");

		REQUIRE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("single job is allocated to single worker", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", { "Job_Dig" });
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(22, "Job_Dig", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(43, "Worker_Builder");

		f.service.allocateJobs();

		REQUIRE(worker.allocatedJobId == job.id);
		REQUIRE(job.allocatedWorkerId == worker.id);
	}

	TEST_CASE("allocateJobs cap assigns only the requested number", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerWorker("Worker_Builder", { "Job_Dig" });
		REQUIRE(f.terrain.digShaft(0));
		REQUIRE(f.terrain.digTile(0, 1));
		REQUIRE(f.terrain.digTile(0, 2));

		JobInstance& firstJob = f.addJob(1, "Job_Dig", glm::ivec2(1, 0));
		JobInstance& secondJob = f.addJob(2, "Job_Dig", glm::ivec2(2, 0));
		WorkerInstance& firstWorker = f.addWorker(10, "Worker_Builder");
		WorkerInstance& secondWorker = f.addWorker(11, "Worker_Builder");

		f.service.allocateJobs(1);

		REQUIRE(firstJob.allocatedWorkerId == firstWorker.id);
		REQUIRE(firstWorker.allocatedJobId == firstJob.id);
		REQUIRE(secondJob.allocatedWorkerId == 0);
		REQUIRE(secondWorker.allocatedJobId == 0);
	}

	TEST_CASE("everyoneCanPerform restore job is taken only while seeking that need", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		JobPrototype sleepJob{};
		sleepJob.name = "Job_Sleep";
		sleepJob.everyoneCanPerform = true;
		sleepJob.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.registerJob(std::move(sleepJob));
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(1, "Job_Sleep", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Miner");

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));

		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;
		REQUIRE(f.service.canWorkerPerformJob(worker, job));

		f.service.allocateJobs();
		REQUIRE(worker.allocatedJobId == job.id);
		REQUIRE(job.allocatedWorkerId == worker.id);
	}

	TEST_CASE("idle worker does not take restore job", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		JobPrototype sleepJob{};
		sleepJob.name = "Job_Sleep";
		sleepJob.everyoneCanPerform = true;
		sleepJob.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.registerJob(std::move(sleepJob));
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(1, "Job_Sleep", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Miner");

		f.service.allocateJobs();
		REQUIRE(worker.allocatedJobId == 0);
		REQUIRE(job.allocatedWorkerId == 0);
	}

	TEST_CASE("seeking worker refuses work jobs", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(1, "Job_Mine", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Miner");
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
		f.service.allocateJobs();
		REQUIRE(worker.allocatedJobId == 0);
	}

	TEST_CASE("restore job without everyoneCanPerform still requires worker job list", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		JobPrototype sleepJob{};
		sleepJob.name = "Job_Sleep";
		sleepJob.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.registerJob(std::move(sleepJob));
		f.makeTileReachable(0, 1);
		JobInstance& job = f.addJob(1, "Job_Sleep", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(2, "Worker_Miner");
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, job));
	}

	TEST_CASE("collapsed worker is assigned nothing", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		JobPrototype sleepJob{};
		sleepJob.name = "Job_Sleep";
		sleepJob.everyoneCanPerform = true;
		sleepJob.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.registerJob(std::move(sleepJob));
		f.makeTileReachable(0, 1);
		JobInstance& sleep = f.addJob(1, "Job_Sleep", glm::ivec2(1, 0));
		JobInstance& mine = f.addJob(2, "Job_Mine", glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(3, "Worker_Miner");
		worker.needValues[needIdFromName("Need_Sleep")] = 0.0f;

		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, sleep));
		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, mine));
	}

	TEST_CASE("chosen seek need prefers lower priority", "[drl][JobAllocationService]")
	{
		Fixture f;
		f.registerNeed("Need_Sleep", 25.0f, 0.0f, 0);
		f.registerNeed("Need_Food", 20.0f, 0.0f, 1);
		f.registerWorker("Worker_Miner", { "Job_Mine" });
		JobPrototype sleepJob{};
		sleepJob.name = "Job_Sleep";
		sleepJob.everyoneCanPerform = true;
		sleepJob.needRestore[needIdFromName("Need_Sleep")] = 15.0f;
		f.registerJob(std::move(sleepJob));
		JobPrototype eatJob{};
		eatJob.name = "Job_Eat";
		eatJob.everyoneCanPerform = true;
		eatJob.needRestore[needIdFromName("Need_Food")] = 15.0f;
		f.registerJob(std::move(eatJob));
		REQUIRE(f.terrain.digShaft(0));
		REQUIRE(f.terrain.digTile(0, 1));
		REQUIRE(f.terrain.digTile(0, 2));
		JobInstance& sleep = f.addJob(1, "Job_Sleep", glm::ivec2(1, 0));
		JobInstance& eat = f.addJob(2, "Job_Eat", glm::ivec2(2, 0));
		WorkerInstance& worker = f.addWorker(3, "Worker_Miner");
		worker.needValues[needIdFromName("Need_Sleep")] = 24.0f;
		worker.needValues[needIdFromName("Need_Food")] = 5.0f;

		REQUIRE(f.service.canWorkerPerformJob(worker, sleep));
		REQUIRE_FALSE(f.service.canWorkerPerformJob(worker, eat));
	}

}
}
