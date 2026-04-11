#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/VulkanMappedBuffer.hpp>

namespace hl
{
	class PipelineDrawData;
	class InputManager;

	class UiRoot : public NonCopyable, public EventListener
	{
	public:
		UiRoot(InputManager& inputManager);

		void initialise(VulkanDevice& device);
		void update(float delta);
		void updateGpuResources(uint32_t currentFrame);
		void draw(PipelineDrawData& pdd) const;
		void destroy();

		void OnEvent(const hl::Event& event) override;

		void* getData() const;
		size_t getDataSize() const;
		size_t getVertexCount() const;

	private:
		InputManager& _inputManager;

		std::vector<hl::VertexUi> _vertices;
		std::vector<VulkanMappedBuffer> _mappedBuffers;

		uint32_t _width{ 0 };
		uint32_t _height{ 0 };
	};

}