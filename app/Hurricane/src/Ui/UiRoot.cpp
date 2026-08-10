#include <Ui/UiRoot.hpp>

#include <Ui/UiElement.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <iostream>
#include <Ui/UiLayout.hpp>

constexpr auto MAX_UI_VERTEXES = 4096;

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
			_mappedBuffers.back().create(sizeof(hl::VertexUi2) * MAX_UI_VERTEXES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		}
	}

	void UiRoot::update(float delta)
	{
		_vertices.clear();

		for (auto& e : _elements)
		{
			e->calculatedRect = hur::UiLayout::Calculate(*e, { _width, _height });
		}

		for (auto& e : _elements)
		{
			e->update(delta);
		}
	}

	void UiRoot::drawUi()
	{
		for (auto& e : _elements)
		{
			e->draw(*this, { _width, _height });
		}
	}

	void UiRoot::addQuad(const hur::UiRect& rect, glm::vec4 colour)
	{		
		const float pixel = 6.0f;
		const float TEX_SIZE = 1024.0f;

		auto texCoords = glm::vec4{ pixel, pixel, 1, 1} / TEX_SIZE;

		addQuad(rect, colour, texCoords);
	}

	void UiRoot::addQuad(const hur::UiRect& rect, glm::vec4 colour, glm::vec4 texCoords)
	{
		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x, rect.position.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x, texCoords.y } });
		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x + rect.size.x, rect.position.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y } });
		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x + rect.size.x, rect.position.y + rect.size.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w } });

		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x, rect.position.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x, texCoords.y } });
		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x + rect.size.x, rect.position.y + rect.size.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x + texCoords.z, texCoords.y + texCoords.w } });
		_vertices.push_back(hl::VertexUi2{
			.pos = { rect.position.x, rect.position.y + rect.size.y },
			.color = { colour.r, colour.g, colour.b },
			.texCoord = glm::vec2{ texCoords.x, texCoords.y + texCoords.w } });
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

	void UiRoot::addElement(hur::UiElement* element)
	{
		_elements.push_back(element);
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
		return _vertices.size() * sizeof(hl::VertexUi2);
	}
	size_t UiRoot::getVertexCount() const
	{
		return _vertices.size();
	}
}