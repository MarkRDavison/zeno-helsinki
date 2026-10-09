#pragma once

#include <Entities/Data/TerrainData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>

namespace drl
{

	class TerrainView
	{
	public:
		TerrainView(const TerrainData& terrainData, float originX, float originY, float tileSize);

		void draw(hl::PipelineDrawData& pdd) const;

	private:
		void drawCell(hl::PipelineDrawData& pdd, int column, int level, int frameIndex) const;

		const TerrainData& _terrainData;
		float _originX;
		float _originY;
		float _tileSize;
	};

}
