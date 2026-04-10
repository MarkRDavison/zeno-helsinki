#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Vulkan/VulkanMappedBuffer.hpp>

namespace ui
{

	class UserInterfaceStartEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		UserInterfaceStartEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig);
		~UserInterfaceStartEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager,
			hl::MaterialSystem& materialSystem) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
		void additionalCleanup() override;

		void OnEvent(const hl::Event& event) override;

	private:
		void handleWindowSizeChange(int width, int height);

	private:
		const hl::EngineConfiguration& _engineConfig;
		std::vector<hl::VulkanMappedBuffer> _mappedBuffers;
	};

}