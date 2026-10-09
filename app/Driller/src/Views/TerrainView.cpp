#include <Views/TerrainView.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	namespace
	{
		constexpr int kFrameUndug = 0;
		constexpr int kFrameDug = 1;
		constexpr int kFrameLadder = 16;
		constexpr int kFrameDrill = 32;
	}

	TerrainView::TerrainView(const TerrainData& terrainData, float originX, float originY, float tileSize)
		: _terrainData(terrainData)
		, _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void TerrainView::drawCell(hl::PipelineDrawData& pdd, int column, int level, int frameIndex) const
	{
		const glm::vec3 position(
			_originX + static_cast<float>(column) * _tileSize,
			_originY + static_cast<float>(level) * _tileSize,
			0.0f);

		auto pc = hl::SpritePushConstantObject
		{
			.model = glm::translate(glm::mat4(1.0f), position),
			.size = glm::vec2(_tileSize, _tileSize),
			.frameIndex = frameIndex,
			.cameraIndex = _cameraIndex
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(hl::SpritePushConstantObject),
			&pc);

		auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
		vkCmdBindDescriptorSets(
			pdd.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pdd.pipeline->getPipelineLayout(),
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDraw(pdd.commandBuffer, 6, 1, 0, 0);
	}

	void TerrainView::draw(hl::PipelineDrawData& pdd) const
	{
		for (int i = 0; i < static_cast<int>(_terrainData.rows.size()); ++i)
		{
			const TerrainRow& row = _terrainData.rows[static_cast<unsigned>(i)];

			for (int j = 1; j <= static_cast<int>(row.leftTiles.size()); ++j)
			{
				drawCell(pdd, -j, i, row.leftTiles[static_cast<unsigned>(j - 1)].dugOut ? kFrameDug : kFrameUndug);
			}

			for (int k = 1; k <= static_cast<int>(row.rightTiles.size()); ++k)
			{
				drawCell(pdd, k, i, row.rightTiles[static_cast<unsigned>(k - 1)].dugOut ? kFrameDug : kFrameUndug);
			}
		}

		int shaftI = 0;
		for (; shaftI <= _terrainData.shaftLevel; ++shaftI)
		{
			drawCell(pdd, 0, shaftI, kFrameLadder);
		}

		drawCell(pdd, 0, shaftI, kFrameDrill);
	}

}
