#pragma once

#include <HurricaneConstants.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <Services/ResourceService.hpp>
#include <Services/GameStateService.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/IconRow.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <Ui/UiLayoutAdapters.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <memory>

namespace hl
{
	class FontResource;
	class LogicalResource;
}

namespace hur
{
	class SceneHost;

	class HurricaneGameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		HurricaneGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			GameStateService& gameStateService,
			ResourceService& resourceService,
			SceneHost& sceneHost);
		~HurricaneGameEngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			hl::VulkanDevice& device,
			hl::VulkanSwapChain& swapChain,
			hl::VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			hl::ResourceManager& resourceManager) override;

		void update(uint32_t currentFrame, float delta) override;
		void updateGpuResources(uint32_t currentFrame) override;
		void additionalCleanup() override;

		void OnEvent(const hl::Event& event) override;

	private:
		void handleWindowSizeChange(int width, int height);

		void setGameState(GameState state);
		void transitionFromInitToPlaying();
		void spawnPlayer();
		void buildHud(hl::FontResource* font);
		void updateUi();
		void applyPendingDeath();
		void clearHostiles();
		void goToTitle();
		void refreshOverlay();
		bool overlayVisible() const;

	private:
		const hl::EngineConfiguration& _engineConfig;
		GameState _state{ GameState::INIT };
		hl::ResourceHandle<hl::StorageBufferResource> _spriteSheetSSBOResourceHandle;
		hl::ResourceHandle<hl::LogicalResource> _uiSheetHandle;
		hl::ResourceDefinition _uiSheetDefinition;
		GameStateService& _gameStateService;
		ResourceService& _resourceService;
		SceneHost& _sceneHost;

		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Label> _score;
		std::unique_ptr<hl::ui::IconRow> _lives;
		std::unique_ptr<hl::ui::IconRow> _bombs;
		std::unique_ptr<hl::ui::Label> _status;

		std::unique_ptr<hl::ui::Node> _overlayRoot;
		std::unique_ptr<hl::ui::Panel> _dim;
		std::unique_ptr<hl::ui::Panel> _overlayPanel;
		std::unique_ptr<hl::ui::Label> _overlayHeading;
		std::unique_ptr<hl::ui::Label> _overlayScore;
		std::unique_ptr<hl::ui::Button> _resume;
		std::unique_ptr<hl::ui::Button> _titleButton;

		bool _pendingDeath = false;
		float _respawnTimer = 0.0f;

		int _width;
		int _height;
	};

}
