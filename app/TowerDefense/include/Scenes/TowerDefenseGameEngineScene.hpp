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
#include <helsinki/System/Events/EventListener.hpp>
#include <Services/GameStateService.hpp>
#include <Services/WaveService.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <SceneCatalog.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hl
{
	class FontResource;
	class Camera;
}

namespace tower
{
	class SceneHost;

	class TowerDefenseGameEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		TowerDefenseGameEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost,
			GameStateService& gameState,
			WaveService& wave,
			CreepCatalog& creeps,
			TowerCatalog& towers,
			EntityCatalog& entities,
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
		void OnEvent(const hl::Event& event) override;
	private:
		std::vector<hl::RenderpassInfo> buildRenderpasses() const;
		void spawnScene(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnBoard(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnBlockers(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnTower(hl::ResourceManager& resourceManager, int tx, int tz, const std::string& defId);
		void selectPlaceTool(const std::string& defId);
		void spawnCreep();
		void spawnMarker(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnGhost(hl::ResourceManager& resourceManager);
		void spawnRangeRing(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnPathRibbons(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		std::optional<TileCoord> hoveredTile() const;
		void updateGhost();
		void tryHandleBoardClick();
		void flashInvalid(int tx, int tz);
		void clearInspect();
		void setInspect(hl::Entity* tower);
		void sellInspectedTower();
		void syncInspectRing();
		hl::Entity* towerAt(int tx, int tz) const;
		hl::Entity* blockerAt(int tx, int tz) const;
		bool invalidFlashBlinkOn() const;
		bool isOccupied(int tx, int tz) const;
		bool matchEnded() const;
		bool uiBlocksBoardClick() const;
		void startWave();
		void tickWave(float delta);
		bool boardHasLiveCreep() const;
		void tryClearWave();
		void onCreepLeaked();
		void onCreepKilled();
		void buildHud(hl::FontResource* font);
		void rebuildHud();
		void updateCameraOrbit();
		void updateCameraFollow(float delta);
		hl::Camera* boardCamera() const;
		const TowerDef* selectedTowerDef() const;

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		GameStateService& _gameState;
		WaveService& _wave;
		CreepCatalog& _creeps;
		TowerCatalog& _towers;
		EntityCatalog& _entities;
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
		bool _orbitDragging = false;
		float _orbitStartMouseX = 0.0f;
		float _orbitStartYaw = 0.0f;
		float _cameraDistance = 0.0f;
		float _cameraDistanceTarget = 0.0f;
		std::string _selectedTowerId;
		hl::Entity* _inspectTower = nullptr;

		hl::UiBatch _uiBatch;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<hl::ui::Label> _goldLabel;
		std::unique_ptr<hl::ui::Label> _livesLabel;
		std::unique_ptr<hl::ui::Label> _waveLabel;
		std::unique_ptr<hl::ui::Button> _waveButton;
		std::unique_ptr<hl::ui::Panel> _buildBarPanel;
		std::vector<std::unique_ptr<hl::ui::Button>> _placeButtons;
		std::unique_ptr<hl::ui::Panel> _inspectPanel;
		std::unique_ptr<hl::ui::Label> _inspectLabel;
		std::unique_ptr<hl::ui::Button> _sellButton;

		std::unique_ptr<hl::ui::Node> _overlayRoot;
		std::unique_ptr<hl::ui::Panel> _dim;
		std::unique_ptr<hl::ui::Panel> _overlayPanel;
		std::unique_ptr<hl::ui::Label> _overlayHeading;
		std::unique_ptr<hl::ui::Button> _titleButton;
	};

}
