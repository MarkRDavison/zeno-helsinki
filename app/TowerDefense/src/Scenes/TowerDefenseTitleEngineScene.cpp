#include "Scenes/TowerDefenseTitleEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Ui/Widget.hpp>

namespace tower
{
	TowerDefenseTitleEngineScene::TowerDefenseTitleEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
	}

	TowerDefenseTitleEngineScene::~TowerDefenseTitleEngineScene()
	{
		_sceneHost.onSceneDestroyed();
	}

	void TowerDefenseTitleEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		hl::ResourceContext resourceContext
		{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};

		loadMenuUiSheet(
			resourceManager,
			resourceContext,
			"title_ui_sheet",
			{
				hl::ResourceDefinition::Child{ .name = "white", .type = "texture" },
				hl::ResourceDefinition::Child{ .name = "roboto", .type = "texture" }
			});

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			{ makeMenuUiRenderpass(cameraMatrixResourceId, "title_ui_sheet") });

		buildMenu(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void TowerDefenseTitleEngineScene::buildMenu(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();

		auto& column = _layoutRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 40.0f;
		column.crossAlign = hl::ui::Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		_title = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_title->setText("Tower Defense", 96);
		_title->color = { 1.0f, 0.5f, 0.0f };

		_start = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_start->setText("Start", 64);
		_start->onClick = [this]()
		{
			_sceneHost.goGame();
		};

		_settings = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_settings->setText("Settings", 64);
		_settings->onClick = [this]()
		{
			_sceneHost.goSettings();
		};

		_quit = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_quit->setText("Quit", 64);
		_quit->onClick = [this]()
		{
			_engine.stop();
		};
	}

	void TowerDefenseTitleEngineScene::rebuildAndDraw(float /*delta*/)
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseTitleEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		rebuildAndDraw(delta);
	}

	void TowerDefenseTitleEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseTitleEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}
}
