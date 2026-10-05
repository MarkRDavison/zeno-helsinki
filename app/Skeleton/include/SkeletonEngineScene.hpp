#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>

namespace sk
{

	class SkeletonEngineScene : public hl::EngineScene
	{
	public:
		SkeletonEngineScene(hl::Engine& engine, const hl::EngineConfiguration& engineConfig);
		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
	private:
		std::vector<hl::RenderpassInfo> buildRenderpasses() const;
		void spawnScene(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);

		const hl::EngineConfiguration& _engineConfig;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;
		hl::ResourceHandle<hl::UniformBufferResource> _pointLightsUbo;
		bool _specHeldOff = false;
	};

}
