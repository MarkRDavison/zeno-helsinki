#include <Views/JobView.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	namespace
	{
		constexpr int kFrameJobMarker = 4;
		constexpr float kMarkerSize = 16.0f;
	}

	JobView::JobView(const JobData& jobData, float originX, float originY, float tileSize)
		: _jobData(jobData)
		, _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void JobView::draw(hl::PipelineDrawData& pdd) const
	{
		for (const JobInstance& job : _jobData.jobs)
		{
			const glm::vec2 center(
				_originX + (static_cast<float>(job.tile.x) + 0.5f + job.offset.x) * _tileSize,
				_originY + (static_cast<float>(job.tile.y) + 0.5f + job.offset.y) * _tileSize);
			const glm::vec3 position(
				center.x - kMarkerSize * 0.5f,
				center.y - kMarkerSize * 0.5f,
				0.0f);

			auto pc = hl::SpritePushConstantObject
			{
				.model = glm::translate(glm::mat4(1.0f), position),
				.size = glm::vec2(kMarkerSize, kMarkerSize),
				.frameIndex = kFrameJobMarker
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
	}

}
