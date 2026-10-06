#include "Scenes/TowerDefenseLevelSelectEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <GLFW/glfw3.h>

namespace tower
{
	TowerDefenseLevelSelectEngineScene::TowerDefenseLevelSelectEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		LevelsCatalog& levels
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_levels(levels)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	TowerDefenseLevelSelectEngineScene::~TowerDefenseLevelSelectEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	void TowerDefenseLevelSelectEngineScene::initialise(
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
			"level_select_ui_sheet",
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
			{ makeMenuUiRenderpass(cameraMatrixResourceId, "level_select_ui_sheet") });

		buildMenu(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void TowerDefenseLevelSelectEngineScene::buildMenu(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();

		auto& column = _layoutRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 40.0f;
		column.crossAlign = hl::ui::Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		_heading = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_heading->setText("Select Level", 96);
		_heading->color = { 1.0f, 0.5f, 0.0f };

		for (const auto& entry : _levels.all())
		{
			auto button = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
			button->setText(entry.label, 64);
			const std::string id = entry.id;
			button->onClick = [this, id]()
			{
				_sceneHost.goGame(id);
			};
			_levelButtons.push_back(std::move(button));
		}

		_back = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_back->setText("Back", 64);
		_back->onClick = [this]() { goBack(); };
	}

	void TowerDefenseLevelSelectEngineScene::goBack()
	{
		_sceneHost.goTitle();
	}

	void TowerDefenseLevelSelectEngineScene::rebuildAndDraw(float /*delta*/)
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseLevelSelectEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		rebuildAndDraw(delta);
	}

	void TowerDefenseLevelSelectEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseLevelSelectEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}

	void TowerDefenseLevelSelectEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_ESCAPE)
			{
				goBack();
			}
		}
	}
}
