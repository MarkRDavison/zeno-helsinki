#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Services/JobAllocationService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
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
		JobAllocationService service{ jobData, workerData, terrain, workerPrototypes };

		void registerWorker(const std::string& name, std::unordered_set<std::string> jobs)
		{
			WorkerPrototype prototype{};
			prototype.name = name;
			prototype.validJobPrototypes = std::move(jobs);
			workerPrototypes.registerPrototype(std::move(prototype));
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

}
}
