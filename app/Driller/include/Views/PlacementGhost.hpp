#pragma once

#include <Entities/Data/JobData.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <helsinki/System/glm.hpp>
#include <vector>

namespace drl
{

	inline glm::vec4 placementGhostColor(bool canPlace, bool canAfford)
	{
		if (canPlace && canAfford)
		{
			return glm::vec4(0.35f, 1.0f, 0.35f, 0.5f);
		}

		return glm::vec4(1.0f, 0.35f, 0.35f, 0.5f);
	}

	inline glm::vec4 queuedBuildGhostColor()
	{
		return glm::vec4(1.0f, 0.78f, 0.28f, 0.5f);
	}

	inline std::vector<JobId> queuedBuildGhostJobIds(
		const JobData& jobData,
		const IBuildingPrototypeService& buildingPrototypes)
	{
		std::vector<JobId> ids;
		const JobPrototypeId buildId = jobPrototypeIdFromName("Job_Build_Building");
		for (const JobInstance& job : jobData.jobs)
		{
			if (job.prototypeId == buildId
				&& buildingPrototypes.isPrototypeRegistered(job.additionalPrototypeId))
			{
				ids.push_back(job.id);
			}
		}
		return ids;
	}

}
