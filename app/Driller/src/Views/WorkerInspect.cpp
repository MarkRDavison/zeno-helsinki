#include <Views/WorkerInspect.hpp>
#include <algorithm>

namespace drl
{

	std::string workerInspectDisplayName(std::string_view name, std::string_view prefix)
	{
		if (name.starts_with(prefix))
		{
			return std::string(name.substr(prefix.size()));
		}

		return std::string(name);
	}

	WorkerSpriteBox workerSpriteBox(
		const WorkerInstance& worker,
		float originX,
		float originY,
		float tileSize)
	{
		const float width = kWorkerWidthTiles * tileSize;
		const float height = kWorkerHeightTiles * tileSize;
		return WorkerSpriteBox{
			.left = originX + (worker.position.x + 0.5f) * tileSize - width * 0.5f,
			.top = originY + worker.position.y * tileSize - height,
			.width = width,
			.height = height
		};
	}

	std::optional<WorkerId> pickWorkerAt(
		const WorkerData& workers,
		glm::vec2 worldPos,
		float originX,
		float originY,
		float tileSize)
	{
		std::optional<WorkerId> chosen;
		float bestDistance = 0.0f;

		for (const WorkerInstance& worker : workers.workers)
		{
			const WorkerSpriteBox box = workerSpriteBox(worker, originX, originY, tileSize);
			if (worldPos.x < box.left || worldPos.x >= box.left + box.width
				|| worldPos.y < box.top || worldPos.y >= box.top + box.height)
			{
				continue;
			}

			const glm::vec2 center{ box.left + box.width * 0.5f, box.top + box.height * 0.5f };
			const float distance = glm::length(worldPos - center);
			if (!chosen || distance < bestDistance)
			{
				chosen = worker.id;
				bestDistance = distance;
			}
		}

		return chosen;
	}

	std::optional<WorkerInspectSnapshot> makeWorkerInspectSnapshot(
		WorkerId workerId,
		const WorkerData& workers,
		const IWorkerPrototypeService& workerPrototypes,
		const JobData& jobs,
		const IJobPrototypeService& jobPrototypes,
		const INeedPrototypeService& needPrototypes)
	{
		const auto workerIt = std::find_if(
			workers.workers.begin(),
			workers.workers.end(),
			[workerId](const WorkerInstance& worker)
			{
				return worker.id == workerId;
			});
		if (workerIt == workers.workers.end())
		{
			return std::nullopt;
		}

		const WorkerInstance& worker = *workerIt;
		WorkerInspectSnapshot snapshot{};
		if (workerPrototypes.isPrototypeRegistered(worker.prototypeId))
		{
			snapshot.typeLabel = workerInspectDisplayName(
				workerPrototypes.getPrototype(worker.prototypeId).name,
				"Worker_");
		}

		if (worker.leaving)
		{
			snapshot.activity = "Leaving";
		}
		else
		{
			switch (worker.state)
			{
			case WorkerState::Idle:
				snapshot.activity = "Idle";
				break;
			case WorkerState::MovingToJob:
				snapshot.activity = "Walking to job";
				break;
			case WorkerState::WorkingJob:
				snapshot.activity = "Working";
				break;
			case WorkerState::Wander:
				snapshot.activity = "Wandering";
				break;
			}
		}

		if (worker.allocatedJobId != 0)
		{
			const auto jobIt = std::find_if(
				jobs.jobs.begin(),
				jobs.jobs.end(),
				[&worker](const JobInstance& job)
				{
					return job.id == worker.allocatedJobId;
				});
			if (jobIt != jobs.jobs.end()
				&& jobPrototypes.isPrototypeRegistered(jobIt->prototypeId))
			{
				snapshot.jobLabel = workerInspectDisplayName(
					jobPrototypes.getPrototype(jobIt->prototypeId).name,
					"Job_");
			}
		}

		std::vector<NeedId> needIds = needPrototypes.registeredIds();
		std::sort(
			needIds.begin(),
			needIds.end(),
			[&needPrototypes](NeedId a, NeedId b)
			{
				return needPrototypes.getPrototype(a).order < needPrototypes.getPrototype(b).order;
			});
		for (const NeedId needId : needIds)
		{
			const NeedPrototype& prototype = needPrototypes.getPrototype(needId);
			WorkerInspectNeed row{};
			row.label = prototype.label;
			const auto valueIt = worker.needValues.find(needId);
			if (valueIt != worker.needValues.end())
			{
				row.value = valueIt->second;
			}
			row.band = classifyNeedValue(row.value, prototype);
			snapshot.needs.push_back(std::move(row));
		}

		return snapshot;
	}

}
