#pragma once

#include <Services/BuildingPlacementService.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/UiService.hpp>
#include <Views/PlacementGhost.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/glm.hpp>
#include <string>

namespace drl
{

	class BuildingGhostView
	{
	public:
		BuildingGhostView(float originX, float originY, float tileSize);
		void setCameraIndex(int cameraIndex) { _cameraIndex = cameraIndex; }

		void draw(
			hl::PipelineDrawData& pdd,
			UiState state,
			const std::string& activeBuilding,
			glm::ivec2 hoveredTile,
			const IBuildingPrototypeService& prototypes,
			const IBuildingPlacementService& placement,
			const IEconomyResourceService& economy) const;

	private:
		void drawCell(
			hl::PipelineDrawData& pdd,
			int column,
			int level,
			int frameIndex,
			const glm::vec4& color) const;

		float _originX;
		float _originY;
		float _tileSize;
		int _cameraIndex{ 0 };
	};

}
