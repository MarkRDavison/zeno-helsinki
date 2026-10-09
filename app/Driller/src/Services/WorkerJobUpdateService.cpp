#include <Services/WorkerJobUpdateService.hpp>
#include <algorithm>

namespace drl
{

	WorkerJobUpdateService::WorkerJobUpdateService(
		WorkerData& workerData,
		JobData& jobData,
		ITerrainAlterationService& terrain,
		const IJobPrototypeService& jobPrototypes)
		: _workerData(workerData)
		, _jobData(jobData)
		, _terrain(terrain)
		, _jobPrototypes(jobPrototypes)
	{
	}

	void WorkerJobUpdateService::update(float delta)
	{
		for (WorkerInstance& worker : _workerData.workers)
		{
			if (worker.state != WorkerState::WorkingJob)
			{
				continue;
			}

			updateWorkerJob(delta, worker, _jobData.getJob(worker.allocatedJobId));
		}

		removeCompletedJobs();
	}

	void WorkerJobUpdateService::updateWorkerJob(float delta, WorkerInstance& worker, JobInstance& job)
	{
		job.work -= delta;
		if (job.work > 0.0f)
		{
			return;
		}

		const JobPrototype& prototype = _jobPrototypes.getPrototype(job.prototypeId);
		if (prototype.repeats)
		{
			job.work += prototype.work;
		}
		else
		{
			job.requiresRemoval = true;
			job.allocatedWorkerId = 0;
			worker.allocatedJobId = 0;
			worker.state = WorkerState::Idle;
			worker.idleTime = 0.0f;
			_terrain.getTile(job.tile.y, job.tile.x).jobReserved = false;
		}

		if (prototype.onComplete)
		{
			prototype.onComplete(job);
		}
	}

	void WorkerJobUpdateService::removeCompletedJobs()
	{
		std::erase_if(
			_jobData.jobs,
			[](const JobInstance& job)
			{
				return job.requiresRemoval;
			});
	}

}
