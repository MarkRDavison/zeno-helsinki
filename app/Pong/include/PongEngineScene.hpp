#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <PongConstants.hpp>

namespace pong
{

	class PongEngineScene : public hl::EngineScene
	{
	public:
		PongEngineScene(hl::Engine& engine, const hl::EngineConfiguration& engineConfig);
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
		GameState _state{ GameState::INIT };
	};

}