#include <catch2/catch_test_macros.hpp>
#include <Entities/Data/JobData.hpp>
#include <Entities/Data/ShuttleData.hpp>
#include <Entities/Data/TerrainData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Shuttle.hpp>
#include <Entities/Job.hpp>
#include <Entities/Worker.hpp>
#include <Services/TerrainAlterationService.hpp>
#include <Services/WorkerMovementService.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{
namespace WorkerMovementServiceTests
{

struct Fixture
	{
		TerrainData terrainData;
		JobData jobData;
		WorkerData workerData;
		ShuttleData shuttleData;
		TerrainAlterationService terrain{ terrainData };
		WorkerMovementService service{ workerData, jobData, terrain, shuttleData };

		ShuttleInstance& addShuttle(glm::vec2 position)
		{
			ShuttleInstance& shuttle = shuttleData.shuttles.emplace_back();
			shuttle.position = position;
			shuttle.surfacePosition = kShuttleSurfacePosition;
			return shuttle;
		}

		JobInstance& addJob(long long jobId, long long workerId, glm::ivec2 tile)
		{
			JobInstance& job = jobData.jobs.emplace_back();
			job.id = jobId;
			job.allocatedWorkerId = workerId;
			job.tile = tile;
			return job;
		}

		WorkerInstance& addWorker(long long workerId, glm::vec2 position)
		{
			WorkerInstance& worker = workerData.workers.emplace_back();
			worker.id = workerId;
			worker.position = position;
			return worker;
		}
};

TEST_CASE("idle worker with assigned job id starts moving to job", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addJob(1, 22, glm::ivec2(2, 0));
	WorkerInstance& worker = f.addWorker(22, glm::vec2(0.0f, 1.0f));
	worker.allocatedJobId = 1;

	f.service.updateWorker(0.0f, worker);

	REQUIRE(worker.state == WorkerState::MovingToJob);
}

TEST_CASE("moving worker approaches job then starts working on arrival", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addJob(1, 22, glm::ivec2(2, 0));
	WorkerInstance& worker = f.addWorker(22, glm::vec2(0.0f, 1.0f));
	worker.allocatedJobId = 1;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.state == WorkerState::MovingToJob);
	REQUIRE(worker.position == glm::vec2(1.0f, 1.0f));

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.state == WorkerState::WorkingJob);
	REQUIRE(worker.position == glm::vec2(2.0f, 1.0f));
}

TEST_CASE("worker on a different level walks to the shaft first", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addJob(1, 22, glm::ivec2(2, 0));
	WorkerInstance& worker = f.addWorker(22, glm::vec2(2.0f, 2.0f));
	worker.allocatedJobId = 1;
	worker.state = WorkerState::MovingToJob;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.state == WorkerState::MovingToJob);
	REQUIRE(worker.position == glm::vec2(1.0f, 2.0f));
}

TEST_CASE("wandering worker approaches target and returns to idle on arrival", "[drl][WorkerMovementService]")
{
	Fixture f;
	WorkerInstance& worker = f.addWorker(22, glm::vec2(1.0f, 1.0f));
	worker.state = WorkerState::Wander;
	worker.wanderTarget = glm::vec2(3.0f, 1.0f);
	worker.idleTime = 5.0f;

	f.service.updateWorker(2.0f, worker);

	REQUIRE(worker.state == WorkerState::Wander);
	REQUIRE(worker.position == glm::vec2(2.0f, 1.0f));

	f.service.updateWorker(2.0f, worker);

	REQUIRE(worker.state == WorkerState::Idle);
	REQUIRE(worker.position == glm::vec2(3.0f, 1.0f));
	REQUIRE(worker.idleTime == 0.0f);
}

TEST_CASE("wandering worker with allocated job starts moving to job", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addJob(1, 22, glm::ivec2(2, 0));
	WorkerInstance& worker = f.addWorker(22, glm::vec2(0.0f, 1.0f));
	worker.allocatedJobId = 1;
	worker.state = WorkerState::Wander;
	worker.wanderTarget = glm::vec2(4.0f, 1.0f);

	f.service.updateWorker(0.0f, worker);

	REQUIRE(worker.state == WorkerState::MovingToJob);
	REQUIRE(worker.position == glm::vec2(0.0f, 1.0f));
}

TEST_CASE("idle worker does not wander onto unreachable tiles", "[drl][WorkerMovementService]")
{
	Fixture f;
	WorkerInstance& worker = f.addWorker(22, glm::vec2(0.0f, 1.0f));

	for (int i = 0; i < 50; ++i)
	{
		f.service.updateWorker(1.0f, worker);
	}

	REQUIRE(worker.state == WorkerState::Idle);
	REQUIRE(worker.position == glm::vec2(0.0f, 1.0f));
	REQUIRE(worker.wanderBackoff > 0.0f);
}

TEST_CASE("leaving worker does not follow idle shuttle into the sky", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addShuttle(kShuttleStartingPosition);
	WorkerInstance& worker = f.addWorker(22, glm::vec2(0.0f, 0.0f));
	worker.leaving = true;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.position == glm::vec2(-1.0f, 0.0f));
	REQUIRE(worker.position.y == 0.0f);
}

TEST_CASE("leaving worker walks toward the shuttle", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addShuttle(kShuttleSurfacePosition);
	WorkerInstance& worker = f.addWorker(22, glm::vec2(1.0f, 0.0f));
	worker.leaving = true;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.position == glm::vec2(0.0f, 0.0f));

	f.service.updateWorker(4.0f, worker);

	REQUIRE(worker.position == kShuttleSurfacePosition);
}

TEST_CASE("leaving worker on a different level walks to the shaft first", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addShuttle(kShuttleSurfacePosition);
	WorkerInstance& worker = f.addWorker(22, glm::vec2(2.0f, 2.0f));
	worker.leaving = true;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.position == glm::vec2(1.0f, 2.0f));
}

TEST_CASE("leaving worker does not wander", "[drl][WorkerMovementService]")
{
	Fixture f;
	f.addShuttle(kShuttleSurfacePosition);
	WorkerInstance& worker = f.addWorker(22, glm::vec2(-4.0f, 0.0f));
	worker.leaving = true;
	worker.idleTime = 10.0f;

	f.service.updateWorker(1.0f, worker);

	REQUIRE(worker.state == WorkerState::Idle);
	REQUIRE(worker.position == kShuttleSurfacePosition);
}

}
}
