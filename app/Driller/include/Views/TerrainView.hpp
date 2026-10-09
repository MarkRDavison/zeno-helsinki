#pragma once

#include <Entities/Data/TerrainData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	class TerrainView
	{
	public:
		TerrainView(const TerrainData& terrainData, float originX, float originY, float tileSize);

		void drawFill(hl::PipelineDrawData& pdd, glm::vec2 aabbMin, glm::vec2 aabbMax) const;
		void draw(hl::PipelineDrawData& pdd) const;
		void setCameraIndex(int cameraIndex) { _cameraIndex = cameraIndex; }

		float originX() const { return _originX; }
		float originY() const { return _originY; }
		float tileSize() const { return _tileSize; }

	private:
		void drawCell(hl::PipelineDrawData& pdd, int column, int level, int frameIndex) const;

		const TerrainData& _terrainData;
		float _originX;
		float _originY;
		float _tileSize;
		int _cameraIndex{ 0 };
	};

}
