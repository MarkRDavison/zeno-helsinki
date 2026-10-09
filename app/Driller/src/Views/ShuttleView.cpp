#include <Views/ShuttleView.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	namespace
	{
		constexpr int kAtlasColumns = 16;
	}

	ShuttleView::ShuttleView(
		const ShuttleData& shuttleData,
		const IShuttlePrototypeService& shuttlePrototypes,
		float originX,
		float originY,
		float tileSize)
		: _shuttleData(shuttleData)
		, _shuttlePrototypes(shuttlePrototypes)
		, _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void ShuttleView::drawCell(hl::PipelineDrawData& pdd, float tileX, float tileY, int frameIndex) const
	{
		const glm::vec3 position(
			_originX + tileX * _tileSize,
			_originY + tileY * _tileSize,
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

	void ShuttleView::draw(hl::PipelineDrawData& pdd) const
	{
		for (const ShuttleInstance& shuttle : _shuttleData.shuttles)
		{
			const ShuttlePrototype& prototype = _shuttlePrototypes.getPrototype(shuttle.prototypeId);
			const glm::vec2 offset(
				-(static_cast<float>(prototype.size.x) - 1.0f) / 2.0f,
				0.0f);

			for (int y = 0; y <= prototype.size.y - 1; ++y)
			{
				for (int x = 0; x <= prototype.size.x - 1; ++x)
				{
					const int frameIndex =
						(prototype.texture.x + x) + (prototype.texture.y + y) * kAtlasColumns;
					drawCell(
						pdd,
						shuttle.position.x + offset.x + static_cast<float>(x),
						shuttle.position.y + offset.y - static_cast<float>(y),
						frameIndex);
				}
			}
		}
	}

}
