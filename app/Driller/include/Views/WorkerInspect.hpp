#pragma once

#include <Entities/Data/JobData.hpp>
#include <Entities/Data/WorkerData.hpp>
#include <Entities/Need.hpp>
#include <Entities/Worker.hpp>
#include <Services/JobPrototypeService.hpp>
#include <Services/NeedPrototypeService.hpp>
#include <Services/WorkerPrototypeService.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace drl
{

	inline constexpr float kWorkerWidthTiles = 0.4f;
	inline constexpr float kWorkerHeightTiles = 0.8f;

	struct WorkerSpriteBox
	{
		float left{ 0.0f };
		float top{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
	};

	WorkerSpriteBox workerSpriteBox(
		const WorkerInstance& worker,
		float originX,
		float originY,
		float tileSize);

	struct WorkerInspectNeed
	{
		std::string label;
		float value{ 0.0f };
		NeedBand band{ NeedBand::Ok };
	};

	struct WorkerInspectSnapshot
	{
		std::string typeLabel;
		std::string activity;
		std::string jobLabel;
		std::vector<WorkerInspectNeed> needs;
	};

	std::string workerInspectDisplayName(std::string_view name, std::string_view prefix);

	std::optional<WorkerId> pickWorkerAt(
		const WorkerData& workers,
		glm::vec2 worldPos,
		float originX,
		float originY,
		float tileSize);

	std::optional<WorkerInspectSnapshot> makeWorkerInspectSnapshot(
		WorkerId workerId,
		const WorkerData& workers,
		const IWorkerPrototypeService& workerPrototypes,
		const JobData& jobs,
		const IJobPrototypeService& jobPrototypes,
		const INeedPrototypeService& needPrototypes);

}
