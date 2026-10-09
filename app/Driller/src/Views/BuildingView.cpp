#include <Views/BuildingView.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	namespace
	{
		constexpr int kAtlasColumns = 16;
	}

	BuildingView::BuildingView(
		const BuildingData& buildingData,
		const IBuildingPrototypeService& buildingPrototypes,
		float originX,
		float originY,
		float tileSize)
		: _buildingData(buildingData)
		, _buildingPrototypes(buildingPrototypes)
		, _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void BuildingView::drawCell(hl::PipelineDrawData& pdd, int column, int level, int frameIndex) const
	{
		const glm::vec3 position(
			_originX + static_cast<float>(column) * _tileSize,
			_originY + static_cast<float>(level) * _tileSize,
			0.0f);

		auto pc = hl::SpritePushConstantObject
		{
			.model = glm::translate(glm::mat4(1.0f), position),
			.size = glm::vec2(_tileSize, _tileSize),
			.frameIndex = frameIndex
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

	void BuildingView::draw(hl::PipelineDrawData& pdd) const
	{
		for (const BuildingInstance& building : _buildingData.buildings)
		{
			const BuildingPrototype& prototype = _buildingPrototypes.getPrototype(building.prototypeId);
			for (int y = 0; y < prototype.size.y; ++y)
			{
				for (int x = 0; x < prototype.size.x; ++x)
				{
					const int frameIndex =
						(prototype.texture.x + x) + (prototype.texture.y + y) * kAtlasColumns;
					drawCell(
						pdd,
						building.coordinates.x + x,
						building.coordinates.y + y,
						frameIndex);
				}
			}
		}
	}

}
