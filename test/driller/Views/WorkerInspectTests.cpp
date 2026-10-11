#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Job.hpp>
#include <Entities/Need.hpp>
#include <Entities/Worker.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/PrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <Views/WorkerInspect.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace WorkerInspectTests
{

	struct Fixture
	{
		WorkerData workerData;
		JobData jobData;
		WorkerPrototypeService workers;
		JobPrototypeService jobs;
		NeedPrototypeService needs;

		WorkerInstance& addWorker(WorkerId id, glm::vec2 position)
		{
			WorkerInstance& worker = workerData.workers.emplace_back();
			worker.id = id;
			worker.position = position;
			return worker;
		}
	};

	TEST_CASE("pickWorkerAt misses empty space", "[drl][WorkerInspect]")
	{
		Fixture f;
		f.addWorker(1, glm::vec2(1.0f, 0.0f));
		REQUIRE_FALSE(pickWorkerAt(f.workerData, glm::vec2(0.0f, 0.0f), 0.0f, 0.0f, 64.0f).has_value());
	}

	TEST_CASE("pickWorkerAt hits a worker sprite", "[drl][WorkerInspect]")
	{
		Fixture f;
		f.addWorker(1, glm::vec2(1.0f, 0.0f));
		const glm::vec2 center{ (1.0f + 0.5f) * 64.0f, -0.4f * 64.0f };
		REQUIRE(pickWorkerAt(f.workerData, center, 0.0f, 0.0f, 64.0f) == 1);
	}

	TEST_CASE("pickWorkerAt hits left and right workers", "[drl][WorkerInspect]")
	{
		Fixture f;
		f.addWorker(1, glm::vec2(-2.0f, 0.0f));
		f.addWorker(2, glm::vec2(2.0f, 0.0f));
		const glm::vec2 leftCenter{ (-2.0f + 0.5f) * 64.0f, -0.4f * 64.0f };
		const glm::vec2 rightCenter{ (2.0f + 0.5f) * 64.0f, -0.4f * 64.0f };
		REQUIRE(pickWorkerAt(f.workerData, leftCenter, 0.0f, 0.0f, 64.0f) == 1);
		REQUIRE(pickWorkerAt(f.workerData, rightCenter, 0.0f, 0.0f, 64.0f) == 2);
	}

	TEST_CASE("pickWorkerAt stacked picks nearer center", "[drl][WorkerInspect]")
	{
		Fixture f;
		f.addWorker(1, glm::vec2(1.0f, 0.0f));
		f.addWorker(2, glm::vec2(1.15f, 0.0f));
		const float width = kWorkerWidthTiles * 64.0f;
		const glm::vec2 firstCenter{ (1.0f + 0.5f) * 64.0f, -0.4f * 64.0f };
		const glm::vec2 click = firstCenter + glm::vec2(-width * 0.2f, 0.0f);
		REQUIRE(pickWorkerAt(f.workerData, click, 0.0f, 0.0f, 64.0f) == 1);
	}

	TEST_CASE("snapshot missing worker is empty", "[drl][WorkerInspect]")
	{
		Fixture f;
		REQUIRE_FALSE(makeWorkerInspectSnapshot(
			1,
			f.workerData,
			f.workers,
			f.jobData,
			f.jobs,
			f.needs).has_value());
	}

	TEST_CASE("snapshot lists needs in order with bands", "[drl][WorkerInspect]")
	{
		Fixture f;
		NeedPrototype rec{};
		rec.name = "Need_Recreation";
		rec.label = "Recreation";
		rec.order = 3;
		rec.seekBelow = 10.0f;
		f.needs.registerPrototype(std::move(rec));
		NeedPrototype sleep{};
		sleep.name = "Need_Sleep";
		sleep.label = "Sleep";
		sleep.order = 1;
		sleep.seekBelow = 25.0f;
		sleep.collapseBelow = 0.0f;
		f.needs.registerPrototype(std::move(sleep));
		NeedPrototype food{};
		food.name = "Need_Food";
		food.label = "Food";
		food.order = 2;
		food.seekBelow = 20.0f;
		food.collapseBelow = 0.0f;
		f.needs.registerPrototype(std::move(food));

		WorkerPrototype miner{};
		miner.name = "Worker_Miner";
		f.workers.registerPrototype(std::move(miner));
		WorkerInstance& worker = f.addWorker(8, glm::vec2(1.0f, 0.0f));
		worker.prototypeId = prototypeIdFromName("Worker_Miner");
		worker.needValues[needIdFromName("Need_Sleep")] = 0.0f;
		worker.needValues[needIdFromName("Need_Food")] = 18.0f;
		worker.needValues[needIdFromName("Need_Recreation")] = 50.0f;

		const auto snapshot = makeWorkerInspectSnapshot(
			8,
			f.workerData,
			f.workers,
			f.jobData,
			f.jobs,
			f.needs);
		REQUIRE(snapshot.has_value());
		REQUIRE(snapshot->typeLabel == "Miner");
		REQUIRE(snapshot->activity == "Idle");
		REQUIRE(snapshot->jobLabel.empty());
		REQUIRE(snapshot->needs.size() == 3);
		REQUIRE(snapshot->needs[0].label == "Sleep");
		REQUIRE(snapshot->needs[0].band == NeedBand::Collapse);
		REQUIRE(snapshot->needs[1].label == "Food");
		REQUIRE(snapshot->needs[1].band == NeedBand::Seek);
		REQUIRE(snapshot->needs[2].label == "Recreation");
		REQUIRE(snapshot->needs[2].band == NeedBand::Ok);
	}

	TEST_CASE("snapshot job name when allocated", "[drl][WorkerInspect]")
	{
		Fixture f;
		JobPrototype mine{};
		mine.name = "Job_Mine";
		f.jobs.registerPrototype(std::move(mine));
		WorkerInstance& worker = f.addWorker(4, glm::vec2(1.0f, 0.0f));
		worker.state = WorkerState::WorkingJob;
		worker.allocatedJobId = 11;
		JobInstance& job = f.jobData.jobs.emplace_back();
		job.id = 11;
		job.prototypeId = jobPrototypeIdFromName("Job_Mine");

		const auto snapshot = makeWorkerInspectSnapshot(
			4,
			f.workerData,
			f.workers,
			f.jobData,
			f.jobs,
			f.needs);
		REQUIRE(snapshot.has_value());
		REQUIRE(snapshot->activity == "Working");
		REQUIRE(snapshot->jobLabel == "Mine");
	}

	TEST_CASE("snapshot leaving beats working job", "[drl][WorkerInspect]")
	{
		Fixture f;
		JobPrototype mine{};
		mine.name = "Job_Mine";
		f.jobs.registerPrototype(std::move(mine));
		WorkerInstance& worker = f.addWorker(2, glm::vec2(1.0f, 0.0f));
		worker.state = WorkerState::WorkingJob;
		worker.leaving = true;
		worker.allocatedJobId = 9;
		JobInstance& job = f.jobData.jobs.emplace_back();
		job.id = 9;
		job.prototypeId = jobPrototypeIdFromName("Job_Mine");

		const auto snapshot = makeWorkerInspectSnapshot(
			2,
			f.workerData,
			f.workers,
			f.jobData,
			f.jobs,
			f.needs);
		REQUIRE(snapshot.has_value());
		REQUIRE(snapshot->activity == "Leaving");
		REQUIRE(snapshot->jobLabel == "Mine");
	}

}
}
