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
#include <Services/GameCommandService.hpp>
#include <Services/GameStateService.hpp>
#include <Services/MatchContext.hpp>
#include <Services/TowerBuildService.hpp>
#include <Services/TowerFocusService.hpp>
#include <Services/TowerSelectionService.hpp>
#include <Services/WaveService.hpp>
#include <Services/CreepCatalog.hpp>
#include <Services/TowerCatalog.hpp>
#include <Services/WeaponCatalog.hpp>
#include <Services/ProjectileCatalog.hpp>
#include <Services/StatusCatalog.hpp>
#include <Services/StatusCategoryCatalog.hpp>
#include <Services/EntityCatalog.hpp>
#include <Services/LevelCatalog.hpp>
#include <SceneCatalog.hpp>
#include <memory>
#include <optional>
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
			MatchContext& match,
			WaveService& wave,
			CreepCatalog& creeps,
			TowerCatalog& towers,
			WeaponCatalog& weapons,
			ProjectileCatalog& projectiles,
			StatusCatalog& statuses,
			StatusCategoryCatalog& statusCategories,
			EntityCatalog& entities,
			LevelCatalog& level,
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
		void spawnEntities(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void selectPlaceTool(const std::string& defId);
		void spawnCreep();
		void spawnGhost(hl::ResourceManager& resourceManager);
		void spawnRangeRing(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void syncStatusRings();
		void spawnPathRibbons(hl::ResourceManager& resourceManager, hl::ResourceContext& resourceContext);
		void spawnTargetLineMaterials();
		void syncTargetLines();
		hl::Entity* acquireTargetLine();
		std::optional<TileCoord> hoveredTile() const;
		void updateGhost();
		void tryHandleBoardClick();
		void flashInvalid(int tx, int tz);
		void clearInspect(bool clearFocus);
		void sellInspectedTower();
		void syncInspectRing();
		hl::Entity* selectedTower() const;
		hl::Entity* towerAt(int tx, int tz) const;
		hl::Entity* entityAt(int tx, int tz) const;
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
		MatchContext& _match;
		WaveService& _wave;
		CreepCatalog& _creeps;
		TowerCatalog& _towers;
		WeaponCatalog& _weapons;
		ProjectileCatalog& _projectiles;
		StatusCatalog& _statuses;
		StatusCategoryCatalog& _statusCategories;
		EntityCatalog& _entities;
		LevelCatalog& _level;
		TowerSelectionService _selection;
		TowerFocusService _focus;
		TowerBuildService _build;
		GameCommandService _commands;
		hl::audio::Audio& _audio;
		hl::ResourceHandle<hl::UniformBufferResource> _sunUbo;
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
		std::vector<hl::Entity*> _targetLines;

		hl::UiBatch _uiBatch;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<hl::ui::Label> _goldLabel;
		std::unique_ptr<hl::ui::Label> _livesLabel;
		std::unique_ptr<hl::ui::Label> _waveLabel;
		std::unique_ptr<hl::ui::Button> _waveButton;
		std::unique_ptr<hl::ui::Panel> _buildBarPanel;
		std::vector<std::string> _placeableIds;
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
