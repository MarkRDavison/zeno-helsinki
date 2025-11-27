#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>

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
			hl::ResourceManager& resourceManager,
			hl::MaterialSystem& materialSystem) override;

		void update(uint32_t currentFrame, float delta) override;
	private:
		const hl::EngineConfiguration& _engineConfig;
	};

}