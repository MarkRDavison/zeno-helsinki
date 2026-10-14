#pragma once

#include <World/GameCommand.hpp>
#include <World/WorldData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/glm.hpp>
#include <optional>
#include <vector>

namespace hl
{
	class ResourceManager;
	class TextSystem;
}

namespace slr
{

	class GalaxyView
	{
	public:
		void initialise(hl::TextSystem& textSystem, const WorldData& world, int galaxyCameraIndex);
		void setSelectedSystemId(std::optional<long long> id);
		void setSelectedFleetId(std::optional<long long> id);
		void setSelectedTruckId(std::optional<long long> id);
		std::optional<long long> selectedSystemId() const { return _selectedSystemId; }
		std::optional<long long> selectedFleetId() const { return _selectedFleetId; }
		std::optional<long long> selectedTruckId() const { return _selectedTruckId; }

		void drawGraph(hl::PipelineDrawData& pdd, hl::ResourceManager& resourceManager) const;
		void drawLabels(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const;
		bool drawInspector(
			std::vector<GameCommand>& pendingCommands,
			std::optional<long long>& openSystemRequested);
		void drawBattleReport();

	private:
		struct Label
		{
			int textId{ -1 };
			glm::vec2 worldCenter{ 0.0f };
		};

		const WorldData* _world{ nullptr };
		int _galaxyCameraIndex{ 0 };
		std::vector<Label> _labels;
		std::optional<long long> _selectedSystemId;
		std::optional<long long> _selectedFleetId;
		std::optional<long long> _selectedTruckId;
		std::optional<long long> _dismissedReportId;
	};

}
