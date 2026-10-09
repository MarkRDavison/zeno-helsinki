#pragma once

#include <Entities/Data/BuildingData.hpp>
#include <Services/BuildingPrototypeService.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>

namespace drl
{

	class BuildingView
	{
	public:
		BuildingView(
			const BuildingData& buildingData,
			const IBuildingPrototypeService& buildingPrototypes,
			float originX,
			float originY,
			float tileSize);

		void draw(hl::PipelineDrawData& pdd) const;
		void setCameraIndex(int cameraIndex) { _cameraIndex = cameraIndex; }

	private:
		void drawCell(hl::PipelineDrawData& pdd, int column, int level, int frameIndex) const;

		const BuildingData& _buildingData;
		const IBuildingPrototypeService& _buildingPrototypes;
		float _originX;
		float _originY;
		float _tileSize;
		int _cameraIndex{ 0 };
	};

}
