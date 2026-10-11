#include <Views/BuildingInspect.hpp>
#include <Views/WorkerInspect.hpp>
#include <algorithm>
#include <unordered_map>

namespace drl
{

	BuildingSpriteBox buildingSpriteBox(
		const BuildingInstance& building,
		const BuildingPrototype& prototype,
		float originX,
		float originY,
		float tileSize)
	{
		const float width = static_cast<float>(std::max(prototype.size.x, 1)) * tileSize;
		const float height = static_cast<float>(std::max(prototype.size.y, 1)) * tileSize;
		return BuildingSpriteBox{
			.left = originX + static_cast<float>(building.coordinates.x) * tileSize,
			.top = originY + static_cast<float>(building.coordinates.y) * tileSize,
			.width = width,
			.height = height
		};
	}

	bool buildingContainsTile(
		const BuildingInstance& building,
		const BuildingPrototype& prototype,
		glm::ivec2 tile)
	{
		return tile.x >= building.coordinates.x
			&& tile.x < building.coordinates.x + prototype.size.x
			&& tile.y >= building.coordinates.y
			&& tile.y < building.coordinates.y + prototype.size.y;
	}

	std::optional<BuildingId> pickPlacedBuildingAt(
		const BuildingData& buildings,
		const IBuildingPrototypeService& buildingPrototypes,
		glm::ivec2 tile)
	{
		for (const BuildingInstance& building : buildings.buildings)
		{
			const BuildingPrototype& prototype = buildingPrototypes.getPrototype(building.prototypeId);
			if (buildingContainsTile(building, prototype, tile))
			{
				return building.id;
			}
		}

		return std::nullopt;
	}

	std::optional<BuildingInspectSnapshot> makeBuildingInspectSnapshot(
		BuildingId buildingId,
		const BuildingData& buildings,
		const IBuildingPrototypeService& buildingPrototypes,
		const JobData& jobs,
		const IJobPrototypeService& jobPrototypes)
	{
		const BuildingInstance* found = nullptr;
		for (const BuildingInstance& building : buildings.buildings)
		{
			if (building.id == buildingId)
			{
				found = &building;
				break;
			}
		}

		if (found == nullptr)
		{
			return std::nullopt;
		}

		const BuildingPrototype& prototype = buildingPrototypes.getPrototype(found->prototypeId);
		BuildingInspectSnapshot snapshot{};
		snapshot.label = prototype.label.empty() ? prototype.name : prototype.label;
		snapshot.beds = buildingMetadataInt(prototype, kBuildingMetadataWorkerCapacity);

		std::unordered_map<JobPrototypeId, std::size_t> jobIndex;
		for (const JobInstance& job : jobs.jobs)
		{
			if (!buildingContainsTile(*found, prototype, job.tile))
			{
				continue;
			}

			const auto existing = jobIndex.find(job.prototypeId);
			if (existing == jobIndex.end())
			{
				const JobPrototype& jobPrototype = jobPrototypes.getPrototype(job.prototypeId);
				jobIndex.insert({ job.prototypeId, snapshot.jobs.size() });
				snapshot.jobs.push_back(BuildingInspectJob{
					.label = workerInspectDisplayName(jobPrototype.name, "Job_"),
					.inUse = job.allocatedWorkerId != 0 ? 1 : 0,
					.total = 1
				});
			}
			else
			{
				BuildingInspectJob& row = snapshot.jobs[existing->second];
				row.total += 1;
				if (job.allocatedWorkerId != 0)
				{
					row.inUse += 1;
				}
			}
		}

		return snapshot;
	}

}
