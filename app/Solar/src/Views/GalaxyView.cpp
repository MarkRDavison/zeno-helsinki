#include <Views/GalaxyView.hpp>
#include <GalaxyPushConstantObject.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/glm.hpp>
#include <imgui.h>
#include <cmath>
#include <string>

namespace slr
{
namespace
{
	const glm::vec4 NodeColour{ 0.75f, 0.85f, 1.0f, 1.0f };
	const glm::vec4 SelectedNodeColour{ 1.0f, 0.95f, 0.75f, 1.0f };
	const glm::vec4 LaneColour{ 0.28f, 0.36f, 0.48f, 1.0f };

	void drawUnitQuad(
		hl::PipelineDrawData& pdd,
		const hl::VertexArrayResource& quad,
		const glm::mat4& model,
		const glm::vec4& color,
		int cameraIndex)
	{
		GalaxyPushConstantObject pc{
			.model = model,
			.color = color,
			.cameraIndex = cameraIndex
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(GalaxyPushConstantObject),
			&pc);

		VkBuffer vertexBuffers[] = { quad._vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);

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

	void drawSdfText(
		hl::PipelineDrawData& pdd,
		hl::TextSystem& textSystem,
		int textId,
		glm::vec2 pos,
		int cameraIndex,
		const glm::vec4& colour)
	{
		const auto& t = textSystem.getText(textId);
		if (t._fontType != hl::FontType::SignedDistanceField)
		{
			return;
		}

		auto pc = hl::TextPushConstantObject{
			.model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f)),
			.colour = colour,
			.fontAtlasIndex = textSystem.getFontAtlasIndex(hl::FontType::SignedDistanceField, "roboto"),
			.cameraIndex = cameraIndex
		};

		vkCmdPushConstants(
			pdd.commandBuffer,
			pdd.pipeline->getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(hl::TextPushConstantObject),
			&pc);

		VkBuffer vertexBuffers[] = { t._vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);

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

		vkCmdDraw(pdd.commandBuffer, t._vertexCount, 1, 0, 0);
	}
}

	void GalaxyView::initialise(hl::TextSystem& textSystem, const WorldData& world, int galaxyCameraIndex)
	{
		_world = &world;
		_galaxyCameraIndex = galaxyCameraIndex;
		_labels.clear();
		_labels.reserve(world.systems.size());
		_selectedSystemId.reset();

		for (const auto& system : world.systems)
		{
			Label label;
			label.textId = textSystem.registerText(system.name, "roboto", 24);
			label.worldCenter = system.position + glm::vec2(0.0f, GalaxyLabelOffsetY);
			_labels.push_back(label);
		}
	}

	void GalaxyView::setSelectedSystemId(std::optional<long long> id)
	{
		_selectedSystemId = id;
		if (_selectedSystemId.has_value() && _world != nullptr && _world->systemById(*_selectedSystemId) == nullptr)
		{
			_selectedSystemId.reset();
		}
	}

	void GalaxyView::drawGraph(hl::PipelineDrawData& pdd, hl::ResourceManager& resourceManager) const
	{
		if (_world == nullptr)
		{
			return;
		}

		const auto& quad = *resourceManager.GetResource<hl::VertexArrayResource>("unit_quad");

		for (const auto& lane : _world->hyperlanes)
		{
			const StarSystem* from = _world->systemById(lane.a);
			const StarSystem* to = _world->systemById(lane.b);
			if (from == nullptr || to == nullptr)
			{
				continue;
			}

			const glm::vec2 delta = to->position - from->position;
			const float length = glm::length(delta);
			if (length <= 0.0f)
			{
				continue;
			}

			const float angle = std::atan2(delta.y, delta.x);
			const glm::vec2 mid = (from->position + to->position) * 0.5f;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(mid, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(length, GalaxyLaneThickness, 1.0f));
			drawUnitQuad(pdd, quad, model, LaneColour, _galaxyCameraIndex);
		}

		for (const auto& system : _world->systems)
		{
			const bool selected = _selectedSystemId.has_value() && system.id == *_selectedSystemId;
			const glm::mat4 model =
				glm::translate(glm::mat4(1.0f), glm::vec3(system.position, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(GalaxyNodeSize, GalaxyNodeSize, 1.0f));
			drawUnitQuad(
				pdd,
				quad,
				model,
				selected ? SelectedNodeColour : NodeColour,
				_galaxyCameraIndex);
		}
	}

	void GalaxyView::drawLabels(hl::PipelineDrawData& pdd, hl::TextSystem& textSystem) const
	{
		for (const auto& label : _labels)
		{
			const auto& size = textSystem.getTextSize(label.textId);
			glm::vec2 pos = label.worldCenter;
			pos.x += size.x - size.z / 2.0f;
			pos.y += size.y - size.w / 2.0f;
			drawSdfText(
				pdd,
				textSystem,
				label.textId,
				pos,
				_galaxyCameraIndex,
				glm::vec4(0.9f, 0.92f, 0.95f, 1.0f));
		}
	}

	bool GalaxyView::drawInspector() const
	{
		if (!_selectedSystemId.has_value() || _world == nullptr)
		{
			return true;
		}

		const StarSystem* system = _world->systemById(*_selectedSystemId);
		if (system == nullptr)
		{
			return false;
		}

		bool open = true;
		ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("System", &open))
		{
			ImGui::TextUnformatted(system->name.c_str());
			ImGui::TextUnformatted(("id " + std::to_string(system->id)).c_str());
		}
		ImGui::End();
		return open;
	}

}
