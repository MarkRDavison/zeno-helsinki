#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>

namespace hur
{

	class HurricaneTitleEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		HurricaneTitleEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig);
		~HurricaneTitleEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager,
			hl::MaterialSystem& materialSystem) override;

		void update(uint32_t currentFrame, float delta) override;

		void OnEvent(const hl::Event& event) override;

	private:
		void handleWindowSizeChange(int width, int height);
		void handleTextClicked(const std::string& name);

	private:
		const hl::EngineConfiguration& _engineConfig;
	};

}