#pragma once

#include <helsinki/Engine/EngineScene.hpp>

namespace sk
{

	class SkeletonEngineScene : public hl::EngineScene
	{
	public:
		SkeletonEngineScene(const std::string& rootPath);
		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager,
			hl::MaterialSystem& materialSystem) override;
	private:
		const std::string _rootPath;
	};

}