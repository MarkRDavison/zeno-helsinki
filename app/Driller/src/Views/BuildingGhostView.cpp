#include <Views/BuildingGhostView.hpp>
#include <Views/PlacementGhost.hpp>
#include <Services/PrototypeService.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>

namespace drl
{

	namespace
	{
		constexpr int kAtlasColumns = 16;
	}

	BuildingGhostView::BuildingGhostView(float originX, float originY, float tileSize)
		: _originX(originX)
		, _originY(originY)
		, _tileSize(tileSize)
	{
	}

	void BuildingGhostView::drawCell(
		hl::PipelineDrawData& pdd,
		int column,
		int level,
		int frameIndex,
		const glm::vec4& color) const
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
			.cameraIndex = _cameraIndex,
			.color = color
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

	void BuildingGhostView::drawPrototypeFootprint(
		hl::PipelineDrawData& pdd,
		const BuildingPrototype& prototype,
		glm::ivec2 origin,
		const glm::vec4& color) const
	{
		for (int y = 0; y < prototype.size.y; ++y)
		{
			for (int x = 0; x < prototype.size.x; ++x)
			{
				const int frameIndex =
					(prototype.texture.x + x) + (prototype.texture.y + y) * kAtlasColumns;
				drawCell(
					pdd,
					origin.x + x,
					origin.y + y,
					frameIndex,
					color);
			}
		}
	}

	void BuildingGhostView::draw(
		hl::PipelineDrawData& pdd,
		UiState state,
		const std::string& activeBuilding,
		glm::ivec2 hoveredTile,
		const IBuildingPrototypeService& prototypes,
		const IBuildingPlacementService& placement,
		const IEconomyResourceService& economy) const
	{
		if (state != UiState::PlacingBuilding || hoveredTile.y < 0 || activeBuilding.empty())
		{
			return;
		}

		const long long prototypeId = prototypeIdFromName(activeBuilding);
		if (!prototypes.isPrototypeRegistered(prototypeId))
		{
			return;
		}

		const BuildingPrototype& prototype = prototypes.getPrototype(prototypeId);
		const bool canPlace = placement.canPlacePrototype(prototypeId, hoveredTile.y, hoveredTile.x);
		const bool canAfford = economy.canAfford(ResourceMoney, prototype.cost);
		const glm::vec4 color = placementGhostColor(canPlace, canAfford);
		drawPrototypeFootprint(pdd, prototype, hoveredTile, color);
	}

	void BuildingGhostView::drawQueued(
		hl::PipelineDrawData& pdd,
		const JobData& jobData,
		const IBuildingPrototypeService& prototypes) const
	{
		const glm::vec4 color = queuedBuildGhostColor();
		for (const JobId jobId : queuedBuildGhostJobIds(jobData, prototypes))
		{
			const JobInstance& job = jobData.getJob(jobId);
			drawPrototypeFootprint(
				pdd,
				prototypes.getPrototype(job.additionalPrototypeId),
				job.tile,
				color);
		}
	}

}
