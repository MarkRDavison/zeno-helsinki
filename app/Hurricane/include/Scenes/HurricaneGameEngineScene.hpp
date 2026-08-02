#pragma once

#include <HurricaneConstants.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <Services/ResourceService.hpp>
#include <Services/GameStateService.hpp>

namespace hur
{

	class HurricaneGameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		HurricaneGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig);
		~HurricaneGameEngineScene();

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

		void setGameState(GameState state);
		void transitionFromInitToPlaying();
		void spawnPlayer();
		void transitionFromGameOverToInit();

	private:
		const hl::EngineConfiguration& _engineConfig;
		GameState _state{ GameState::INIT };
		hl::ResourceHandle<hl::StorageBufferResource> _spriteSheetSSBOResourceHandle;
		GameStateService _gameStateService; // TODO: Move to service provider and inject???
		ResourceService _resourceService; // TODO: Move to service provider and inject???
		// TODO: Maybe also inject systems?
	};

}