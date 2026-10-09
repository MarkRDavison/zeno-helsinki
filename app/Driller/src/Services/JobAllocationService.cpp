#include <Services/JobAllocationService.hpp>

namespace drl
{

	JobAllocationService::JobAllocationService(
		JobData& jobData,
		WorkerData& workerData,
		const ITerrainAlterationService& terrain,
		const IWorkerPrototypeService& workerPrototypes)
		: _jobData(jobData)
		, _workerData(workerData)
		, _terrain(terrain)
		, _workerPrototypes(workerPrototypes)
	{
	}

	void JobAllocationService::update(float /*delta*/)
	{
		allocateJobs(10);
	}

	void JobAllocationService::allocateJobs(int number)
	{
		int allocated = 0;
		for (JobInstance& job : _jobData.jobs)
		{
			if (job.allocatedWorkerId != 0)
			{
				continue;
			}

			for (WorkerInstance& worker : _workerData.workers)
			{
				if (worker.allocatedJobId != 0)
				{
					continue;
				}

				if (!canWorkerPerformJob(worker, job))
				{
					continue;
				}

				worker.allocatedJobId = job.id;
				job.allocatedWorkerId = worker.id;
				++allocated;
				if (number >= 0 && allocated >= number)
				{
					return;
				}
				break;
			}
		}
	}

	bool JobAllocationService::canWorkerPerformJob(const WorkerInstance& worker, const JobInstance& job) const
	{
		if (!_workerPrototypes.isPrototypeRegistered(worker.prototypeId))
		{
			return false;
		}

		const WorkerPrototype& workerPrototype = _workerPrototypes.getPrototype(worker.prototypeId);
		bool valid = false;
		for (const std::string& jobName : workerPrototype.validJobPrototypes)
		{
			if (_workerPrototypes.getPrototypeId(jobName) == job.prototypeId)
			{
				valid = true;
				break;
			}
		}

		if (!valid)
		{
			return false;
		}

		if (!_terrain.canTileBeReached(job.tile.y, job.tile.x))
		{
			return false;
		}

		return true;
	}

}
