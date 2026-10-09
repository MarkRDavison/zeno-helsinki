#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Scripting/PrototypeBindings.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerJobUpdateService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/Scripting/LuaState.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace WorkerJobUpdateServiceTests
{

	struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		WorkerData workerData;
		TerrainAlterationService terrain{ terrainData };
		JobPrototypeService jobPrototypes;
		WorkerJobUpdateService service{ workerData, jobData, terrain, jobPrototypes };

		void registerJob(const std::string& name, float work, bool repeats)
		{
			JobPrototype prototype{};
			prototype.name = name;
			prototype.work = work;
			prototype.repeats = repeats;
			jobPrototypes.registerPrototype(std::move(prototype));
		}

		JobInstance& addJob(long long id, const std::string& prototypeName, float work, glm::ivec2 tile)
		{
			JobInstance& job = jobData.jobs.emplace_back();
			job.id = id;
			job.prototypeId = jobPrototypeIdFromName(prototypeName);
			job.work = work;
			job.tile = tile;
			return job;
		}

		WorkerInstance& addWorker(long long id, long long jobId)
		{
			WorkerInstance& worker = workerData.workers.emplace_back();
			worker.id = id;
			worker.allocatedJobId = jobId;
			worker.state = WorkerState::WorkingJob;
			return worker;
		}

		void reserveTile(int level, int column)
		{
			REQUIRE(terrain.digShaft(level));
			terrain.initialiseTile(level, column);
			terrain.getTile(level, column).jobReserved = true;
		}
	};

	TEST_CASE("updateWorkerJob decreases work remaining", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		f.registerJob("Job_Dig", 3.0f, false);
		f.reserveTile(0, 1);
		JobInstance& job = f.addJob(1, "Job_Dig", 3.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;

		f.service.updateWorkerJob(1.0f, worker, job);
		REQUIRE(job.work == 2.0f);
		f.service.updateWorkerJob(1.0f, worker, job);
		REQUIRE(job.work == 1.0f);
		f.service.updateWorkerJob(1.0f, worker, job);
		REQUIRE(job.work == 0.0f);
	}

	TEST_CASE("non-repeat job completion unassigns worker and clears reserve", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		f.registerJob("Job_Dig", 3.0f, false);
		f.reserveTile(0, 1);
		JobInstance& job = f.addJob(1, "Job_Dig", 3.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;
		worker.idleTime = 4.0f;

		f.service.updateWorkerJob(3.0f, worker, job);

		REQUIRE(job.requiresRemoval);
		REQUIRE(job.allocatedWorkerId == 0);
		REQUIRE(worker.allocatedJobId == 0);
		REQUIRE(worker.state == WorkerState::Idle);
		REQUIRE(worker.idleTime == 0.0f);
		REQUIRE_FALSE(f.terrain.getTile(0, 1).jobReserved);
	}

	TEST_CASE("repeating job keeps worker and refills work", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		f.registerJob("Job_Mine", 3.0f, true);
		JobInstance& job = f.addJob(1, "Job_Mine", 3.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;

		f.service.updateWorkerJob(1.5f, worker, job);
		REQUIRE(job.work == 1.5f);
		f.service.updateWorkerJob(1.5f, worker, job);

		REQUIRE(job.work == 3.0f);
		REQUIRE(worker.allocatedJobId == job.id);
		REQUIRE(job.allocatedWorkerId == worker.id);
		REQUIRE_FALSE(job.requiresRemoval);
	}

	TEST_CASE("onComplete runs when a non-repeat job completes", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		bool invoked = false;
		JobPrototype prototype{};
		prototype.name = "Job_Dig";
		prototype.work = 3.0f;
		prototype.repeats = false;
		prototype.onComplete = [&invoked](const JobInstance&)
		{
			invoked = true;
		};
		f.jobPrototypes.registerPrototype(std::move(prototype));
		f.reserveTile(0, 1);
		JobInstance& job = f.addJob(1, "Job_Dig", 3.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;

		f.service.updateWorkerJob(3.0f, worker, job);

		REQUIRE(invoked);
	}

	TEST_CASE("onComplete runs each time a repeating job completes", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		int invokedCount = 0;
		JobPrototype prototype{};
		prototype.name = "Job_Mine";
		prototype.work = 3.0f;
		prototype.repeats = true;
		prototype.onComplete = [&invokedCount](const JobInstance&)
		{
			++invokedCount;
		};
		f.jobPrototypes.registerPrototype(std::move(prototype));
		JobInstance& job = f.addJob(1, "Job_Mine", 3.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;

		f.service.updateWorkerJob(3.0f, worker, job);
		REQUIRE(invokedCount == 1);
		f.service.updateWorkerJob(3.0f, worker, job);
		REQUIRE(invokedCount == 2);
		f.service.updateWorkerJob(3.0f, worker, job);
		REQUIRE(invokedCount == 3);
	}

	TEST_CASE("removeCompletedJobs erases only flagged jobs", "[drl][WorkerJobUpdateService]")
	{
		Fixture f;
		f.jobData.jobs.emplace_back().requiresRemoval = false;
		f.jobData.jobs.emplace_back().requiresRemoval = true;
		f.jobData.jobs.emplace_back().requiresRemoval = false;
		f.jobData.jobs.emplace_back().requiresRemoval = true;

		f.service.removeCompletedJobs();

		REQUIRE(f.jobData.jobs.size() == 2);
		REQUIRE_FALSE(f.jobData.jobs[0].requiresRemoval);
		REQUIRE_FALSE(f.jobData.jobs[1].requiresRemoval);
	}

	TEST_CASE("Lua onComplete error does not throw and still completes the job", "[drl][WorkerJobUpdateService]")
	{
		hl::scripting::LuaState lua;
		Fixture f;
		WorkerPrototypeService workers;
		BuildingPrototypeService buildings;
		bindPrototypeUserTypes(lua.raw());
		lua.runString(R"(
	prototypes = {
		jobs = {
			{
				name = "Job_Dig",
				repeats = false,
				work = 1.0,
				onComplete = function(job)
					error("boom")
				end
			}
		},
		workers = {},
		buildings = {}
	}
	)", "onComplete-error");
		applyPrototypesTable(lua.raw()["prototypes"], f.jobPrototypes, workers, buildings);
		f.reserveTile(0, 1);
		JobInstance& job = f.addJob(1, "Job_Dig", 1.0f, glm::ivec2(1, 0));
		WorkerInstance& worker = f.addWorker(22, job.id);
		job.allocatedWorkerId = worker.id;

		REQUIRE_NOTHROW(f.service.updateWorkerJob(1.0f, worker, job));
		REQUIRE(job.requiresRemoval);
		REQUIRE(worker.state == WorkerState::Idle);
		REQUIRE(worker.allocatedJobId == 0);
	}

}
}
