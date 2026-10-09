#include <Views/WorkerView.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/glm.hpp>

namespace drl
{

	namespace
	{
		constexpr int kFrameWorker = 17;
		constexpr float kWorkerWidthTiles = 0.4f;
		constexpr float kWorkerHeightTiles = 0.8f;
	}

	WorkerView::WorkerView(const WorkerData& workerData, float originX, float originY, float tileSize)
		: _workerData(workerData)
		, _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void WorkerView::draw(hl::PipelineDrawData& pdd) const
	{
		const float width = kWorkerWidthTiles * _tileSize;
		const float height = kWorkerHeightTiles * _tileSize;

		for (const WorkerInstance& worker : _workerData.workers)
		{
			const glm::vec3 position(
				_originX + (worker.position.x + 0.5f) * _tileSize - width * 0.5f,
				_originY + worker.position.y * _tileSize - height,
				0.0f);

			auto pc = hl::SpritePushConstantObject
			{
				.model = glm::translate(glm::mat4(1.0f), position),
				.size = glm::vec2(width, height),
				.frameIndex = kFrameWorker,
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
	}

}
