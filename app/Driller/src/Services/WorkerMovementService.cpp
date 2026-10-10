#include <Services/WorkerMovementService.hpp>
#include <helsinki/System/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace drl
{

	namespace
	{
		bool moveTowardsTarget(float speed, float delta, WorkerInstance& worker, glm::vec2 target)
		{
			const float distanceToTarget = glm::length(target - worker.position);
			const float maxMovement = delta * speed;

			if (distanceToTarget <= maxMovement)
			{
				worker.position = target;
				return true;
			}

			worker.position += glm::normalize(target - worker.position) * maxMovement;
			return false;
		}

		glm::vec2 waypointTowards(glm::vec2 position, glm::vec2 destination)
		{
			if (destination.y != position.y)
			{
				if (position.x != 0.0f)
				{
					return { 0.0f, position.y };
				}

				return { 0.0f, destination.y };
			}

			return destination;
		}
	}

	WorkerMovementService::WorkerMovementService(
		WorkerData& workerData,
		const JobData& jobData,
		const ITerrainAlterationService& terrain,
		const ShuttleData& shuttleData)
		: _workerData(workerData)
		, _jobData(jobData)
		, _terrain(terrain)
		, _shuttleData(shuttleData)
	{
	}

	void WorkerMovementService::update(float delta)
	{
		for (WorkerInstance& worker : _workerData.workers)
		{
			updateWorker(delta, worker);
		}
	}

	void WorkerMovementService::updateIdleWorker(float delta, WorkerInstance& worker)
	{
		constexpr float WanderStartTime = 2.5f;
		if (worker.allocatedJobId > 0)
		{
			worker.state = WorkerState::MovingToJob;
			updateWorker(delta, worker);
		}
		else
		{
			worker.idleTime += delta;
			if (worker.idleTime > WanderStartTime * worker.wanderBackoff)
			{
				const int range = 5;
				const int offset = (rand() % range) - (range - 1) / 2;

				if (offset != 0)
				{
					const float pos = worker.position.x + static_cast<float>(offset);
					const float tileX = static_cast<float>(static_cast<int>(pos < 0.0f
						? std::floor(pos + 0.5f)
						: std::ceil(pos - 0.5f)));
					const float tileY = static_cast<float>(static_cast<int>(worker.position.y - 1));

					if (_terrain.canTileBeReached(static_cast<int>(tileY), static_cast<int>(tileX)))
					{
						worker.wanderTarget = { tileX, worker.position.y };
						worker.wanderBackoff = 1.0f;
						worker.state = WorkerState::Wander;
						updateWorker(delta, worker);
					}
					else
					{
						worker.wanderBackoff += 1.0f;
					}
				}
			}
		}
	}

	void WorkerMovementService::updateMovingToJobWorker(float delta, WorkerInstance& worker)
	{
		const auto jobIter = std::find_if(
			_jobData.jobs.begin(),
			_jobData.jobs.end(),
			[&worker](const JobInstance& job)
			{
				return job.allocatedWorkerId == worker.id;
			});

		if (jobIter == _jobData.jobs.end())
		{
			throw std::logic_error("Worker allocated to missing job");
		}

		const JobInstance& job = *jobIter;
		const glm::vec2 jobPos = glm::vec2(job.tile) + glm::vec2(0.0f, 1.0f) + job.offset;
		const glm::vec2 pos = waypointTowards(worker.position, jobPos);

		if (moveTowardsTarget(1.0f, delta, worker, pos) && jobPos == worker.position)
		{
			worker.state = WorkerState::WorkingJob;
		}
	}

	void WorkerMovementService::updateLeavingWorker(float delta, WorkerInstance& worker)
	{
		if (_shuttleData.shuttles.empty())
		{
			return;
		}

		const glm::vec2 destination = _shuttleData.shuttles.front().surfacePosition;
		const glm::vec2 pos = waypointTowards(worker.position, destination);
		moveTowardsTarget(1.0f, delta, worker, pos);
	}

	void WorkerMovementService::updateWanderingWorker(float delta, WorkerInstance& worker)
	{
		if (worker.allocatedJobId != 0)
		{
			updateIdleWorker(delta, worker);
			return;
		}

		if (moveTowardsTarget(0.5f, delta, worker, worker.wanderTarget) && worker.wanderTarget == worker.position)
		{
			worker.state = WorkerState::Idle;
			worker.idleTime = 0.0f;
		}
	}

	void WorkerMovementService::updateWorker(float delta, WorkerInstance& worker)
	{
		if (worker.leaving)
		{
			updateLeavingWorker(delta, worker);
			return;
		}

		switch (worker.state)
		{
		case WorkerState::Idle:
			updateIdleWorker(delta, worker);
			break;
		case WorkerState::MovingToJob:
			updateMovingToJobWorker(delta, worker);
			break;
		case WorkerState::Wander:
			updateWanderingWorker(delta, worker);
			break;
		case WorkerState::WorkingJob:
			break;
		default:
			throw std::runtime_error("Invalid WorkerState");
		}
	}

}
