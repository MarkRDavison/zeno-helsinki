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

}
