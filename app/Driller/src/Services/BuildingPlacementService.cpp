#include <Services/BuildingPlacementService.hpp>
#include <stdexcept>

namespace drl
{

	BuildingPlacementService::BuildingPlacementService(
		BuildingData& buildingData,
		ITerrainAlterationService& terrain,
		IWorkerRecruitmentService& recruitment,
		IJobCreationService& jobs,
		IBuildingPrototypeService& buildingPrototypes)
		: _buildingData(buildingData)
		, _terrain(terrain)
		, _recruitment(recruitment)
		, _jobs(jobs)
		, _buildingPrototypes(buildingPrototypes)
	{
	}

	bool BuildingPlacementService::canPlacePrototype(BuildingPrototypeId prototypeId, int level, int column) const
	{
		if (!_buildingPrototypes.isPrototypeRegistered(prototypeId))
		{
			return false;
		}

		const BuildingPrototype& prototype = _buildingPrototypes.getPrototype(prototypeId);
		const int maxLevel = level + prototype.size.y;
		const int maxCol = column + prototype.size.x;

		for (int y = level; y < maxLevel; ++y)
		{
			for (int x = column; x < maxCol; ++x)
			{
				if (!_terrain.doesTileExist(y, x))
				{
					return false;
				}

				const Tile& tile = _terrain.getTile(y, x);
				if (!tile.dugOut || tile.jobReserved || tile.hasBuilding)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool BuildingPlacementService::placePrototype(BuildingPrototypeId prototypeId, int level, int column)
	{
		if (!canPlacePrototype(prototypeId, level, column))
		{
			return false;
		}

		const BuildingPrototype& prototype = _buildingPrototypes.getPrototype(prototypeId);
		for (const auto& providedJob : prototype.providedJobs)
		{
			if (!_jobs.isNamedPrototypeRegistered(providedJob.first))
			{
				return false;
			}
		}

		_buildingData.buildings.emplace_back(_buildingPrototypes.createInstance(prototypeId));
		BuildingInstance& instance = _buildingData.buildings.back();
		instance.coordinates = glm::ivec2(column, level);

		for (const auto& [workerPrototypeName, amount] : prototype.requiredWorkers)
		{
			_recruitment.registerWorkerPrototypeRequirement(workerPrototypeName, amount);
		}

		for (int y = instance.coordinates.y; y <= instance.coordinates.y + prototype.size.y - 1; ++y)
		{
			for (int x = instance.coordinates.x; x <= instance.coordinates.x + prototype.size.x - 1; ++x)
			{
				_terrain.getTile(y, x).hasBuilding = true;
			}
		}

		for (const auto& providedJob : prototype.providedJobs)
		{
			if (!_jobs.createJob(providedJob.first, providedJob.second, glm::ivec2(column, level)))
			{
				throw std::runtime_error("Failed to create job");
			}
		}

		return true;
	}

}
