#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <Ui/UiLayoutAdapters.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Audio/Audio.hpp>
#include <SceneCatalog.hpp>
#include <memory>

namespace hl
{
	class FontResource;
}

namespace tower
{
	class SceneHost;

	class TowerDefenseGameEngineScene : public hl::EngineScene
	{
	public:
		TowerDefenseGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost,
			hl::audio::Audio& audio);
		~TowerDefenseGameEngineScene();
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
	private:
		std::vector<hl::RenderpassInfo> buildRenderpasses() const;
		void spawnScene(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnBoard(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnTower(hl::ResourceManager& resourceManager, int tx, int tz);
		void spawnCreep();
		void spawnMarker(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnGhost(hl::ResourceManager& resourceManager);
		void spawnRangeRing(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnPathRibbons(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		std::optional<TileCoord> hoveredTile() const;
		void updateGhost();
		void tryHandleBoardClick();
		void flashInvalid(int tx, int tz);
		hl::Entity* towerAt(int tx, int tz) const;
		bool invalidFlashBlinkOn() const;
		bool isOccupied(int tx, int tz) const;
		bool matchEnded() const;
		bool uiBlocksBoardClick() const;
		void startWave();
		void tickWave(float delta);
		void tryWin();
		void onCreepLeaked();
		void onCreepKilled();
		void buildHud(hl::FontResource* font);
		void rebuildHud();

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		hl::audio::Audio& _audio;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;
		hl::Entity* _marker = nullptr;
		hl::Entity* _ghost = nullptr;
		hl::Entity* _rangeRing = nullptr;
		bool _ghostVisible = false;
		bool _ghostPlaceable = false;
		hl::Entity* _flashTower = nullptr;
		bool _flashGhost = false;
		float _invalidFlashRemaining = 0.0f;
		int _gold = StartGold;
		int _lives = StartLives;
		bool _gameOver = false;
		bool _won = false;
		bool _waveStarted = false;
		int _creepsToSpawn = 0;
		float _spawnTimer = 0.0f;

		hl::UiBatch _uiBatch;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<hl::ui::Label> _goldLabel;
		std::unique_ptr<hl::ui::Label> _livesLabel;
		std::unique_ptr<hl::ui::Button> _waveButton;

		std::unique_ptr<hl::ui::Node> _overlayRoot;
		std::unique_ptr<hl::ui::Panel> _dim;
		std::unique_ptr<hl::ui::Panel> _overlayPanel;
		std::unique_ptr<hl::ui::Label> _overlayHeading;
		std::unique_ptr<hl::ui::Button> _titleButton;
	};

}
