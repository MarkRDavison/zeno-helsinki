#pragma once

#include <Views/GalaxyView.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/System/Events/EventListener.hpp>

namespace slr
{

	class GameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		GameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig);
		~GameEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void OnEvent(const hl::Event& event) override;

	private:
		void handleWindowSizeChange(int width, int height);

		const hl::EngineConfiguration& _engineConfig;
		GalaxyView _galaxy;
	};

}
