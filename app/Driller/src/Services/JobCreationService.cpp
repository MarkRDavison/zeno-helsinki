#include <Services/JobCreationService.hpp>

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

}
