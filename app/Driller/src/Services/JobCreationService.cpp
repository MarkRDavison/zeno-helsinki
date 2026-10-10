#include <Services/JobCreationService.hpp>
#include <algorithm>

namespace drl
{

	JobCreationService::JobCreationService(
		JobData& jobData,
		IJobPrototypeService& jobPrototypeService,
		ITerrainAlterationService& terrain)
		: _jobData(jobData)
		, _jobPrototypeService(jobPrototypeService)
		, _terrain(terrain)
	{
	}

	bool JobCreationService::createJob(
		JobPrototypeId prototypeId,
		JobPrototypeId additionalPrototypeId,
		glm::ivec2 coordinates)
	{
		if (!_jobPrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		if (!_terrain.doesTileExist(coordinates.y, coordinates.x))
		{
			return false;
		}

		Tile& tile = _terrain.getTile(coordinates.y, coordinates.x);
		if (tile.jobReserved)
		{
			return false;
		}

		const JobPrototype& prototype = _jobPrototypeService.getPrototype(prototypeId);
		JobInstance& job = _jobData.jobs.emplace_back(_jobPrototypeService.createInstance(prototypeId));
		job.additionalPrototypeId = additionalPrototypeId;
		job.tile = coordinates;
		tile.jobReserved = true;

		if (prototype.calculateOffset)
		{
			job.offset = prototype.calculateOffset(job, prototype);
		}

		return true;
	}

	bool JobCreationService::createJob(
		const std::string& prototypeName,
		glm::vec2 offset,
		glm::ivec2 coordinates)
	{
		const JobPrototypeId prototypeId = jobPrototypeIdFromName(prototypeName);
		if (!_jobPrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		JobInstance& job = _jobData.jobs.emplace_back(_jobPrototypeService.createInstance(prototypeId));
		job.additionalPrototypeId = 0;
		job.tile = coordinates;
		job.offset = offset;
		return true;
	}

	bool JobCreationService::isNamedPrototypeRegistered(const std::string& prototypeName) const
	{
		return _jobPrototypeService.isPrototypeRegistered(jobPrototypeIdFromName(prototypeName));
	}

	bool JobCreationService::isRestoreJob(const std::string& prototypeName) const
	{
		const JobPrototypeId prototypeId = jobPrototypeIdFromName(prototypeName);
		if (!_jobPrototypeService.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		return !_jobPrototypeService.getPrototype(prototypeId).needRestore.empty();
	}

	bool JobCreationService::cancelNonRepeatingJobs(
		glm::ivec2 coordinates,
		std::vector<JobInstance>& cancelled)
	{
		cancelled.clear();
		std::vector<JobId> ids;
		for (const JobInstance& job : _jobData.jobs)
		{
			if (job.tile == coordinates)
			{
				ids.push_back(job.id);
			}
		}

		if (ids.empty())
		{
			return false;
		}

		for (const JobId id : ids)
		{
			const JobInstance& job = _jobData.getJob(id);
			if (_jobPrototypeService.getPrototype(job.prototypeId).repeats)
			{
				return false;
			}
		}

		for (const JobId id : ids)
		{
			const JobInstance job = _jobData.getJob(id);
			cancelled.push_back(job);
			if (_terrain.doesTileExist(job.tile.y, job.tile.x))
			{
				_terrain.getTile(job.tile.y, job.tile.x).jobReserved = false;
			}
		}

		std::erase_if(
			_jobData.jobs,
			[&](const JobInstance& job)
			{
				return std::find(ids.begin(), ids.end(), job.id) != ids.end();
			});
		return true;
	}

}
