#include <Candidates/UiRoot.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <iostream>

constexpr auto MAX_UI_VERTEXES = 1024;

namespace hl
{
	UiRoot::UiRoot(
		InputManager& inputManager
	) :
		_inputManager(inputManager)
	{

	}

	void UiRoot::initialise(VulkanDevice& device)
	{
		const auto size = _inputManager.getWindowSize();
		_width = size.x;
		_height = size.y;
		for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			_mappedBuffers.emplace_back(device);
			_mappedBuffers.back().create(sizeof(hl::VertexUi) * MAX_UI_VERTEXES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}

	}
	void UiRoot::update(float delta)
	{
		const auto mouse = _inputManager.getMousePosition();
		const auto margin = 16.0f;

		_vertices = std::vector<hl::VertexUi>
		{
			hl::VertexUi{.pos = { margin, margin }, .color = { 0.7, 0.7, 0.7 } },
			hl::VertexUi{.pos = { _width - margin, margin }, .color = { 0.7, 0.7, 0.7 } },
			hl::VertexUi{.pos = { _width - margin, _height - margin }, .color = { 0.7, 0.7, 0.7 } },

			hl::VertexUi{.pos = { margin, margin }, .color = { 0.7, 0.7, 0.7 } },
			hl::VertexUi{.pos = { _width - margin, _height - margin }, .color = { 0.7, 0.7, 0.7 } },
			hl::VertexUi{.pos = { margin, _height - margin }, .color = { 0.7, 0.7, 0.7 } },

			hl::VertexUi{.pos = { -50.0f + mouse.x, +50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },
			hl::VertexUi{.pos = { +50.0f + mouse.x, +50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },
			hl::VertexUi{.pos = { +50.0f + mouse.x, -50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },

			hl::VertexUi{.pos = { -50.0f + mouse.x, +50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },
			hl::VertexUi{.pos = { +50.0f + mouse.x, -50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },
			hl::VertexUi{.pos = { -50.0f + mouse.x, -50.0f + mouse.y }, .color = { 1.0, 1.0, 0.0 } },
		};
	}
	void UiRoot::updateGpuResources(uint32_t currentFrame)
	{
		const auto size = getDataSize();

		if (size > 0)
		{
			_mappedBuffers[currentFrame].write(getData(), size);
		}
	}
	void UiRoot::draw(PipelineDrawData& pdd) const
	{
		const auto vertexCount = getVertexCount();

		if (vertexCount > 0)
		{
			VkBuffer vertexBuffers[] = { _mappedBuffers[pdd.currentFrame].getBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(
				pdd.commandBuffer,
				0,
				1,
				vertexBuffers,
				offsets);

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

			vkCmdDraw(pdd.commandBuffer, vertexCount, 1, 0, 0);
		}
	}
	void UiRoot::destroy()
	{
		for (auto& mb : _mappedBuffers)
		{
			mb.destroy();
		}

		_mappedBuffers.clear();
	}
	
	void UiRoot::OnEvent(const hl::Event& event)
	{
		if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			_width = wre->GetWidth();
			_height = wre->GetHeight();
		}
	}

	void* UiRoot::getData() const
	{
		return (void*)_vertices.data();
	}
	size_t UiRoot::getDataSize() const
	{
		return _vertices.size() * sizeof(hl::VertexUi);
	}
	size_t UiRoot::getVertexCount() const
	{
		return _vertices.size();
	}
}