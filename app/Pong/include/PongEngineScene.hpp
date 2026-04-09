#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <PongConstants.hpp>

namespace pong
{

	class PongEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		PongEngineScene(
			hl::Engine& engine, 
			const hl::EngineConfiguration& engineConfig);
		~PongEngineScene();
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

	private:
		const hl::EngineConfiguration& _engineConfig;
		GameState _state{ GameState::INIT };

		int _player1Score{ 0 };
		int _player2Score{ 0 };
	};

}