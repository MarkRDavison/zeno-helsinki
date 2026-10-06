#include "Scenes/TowerDefenseCampaignHubEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <Services/CampaignEvaluator.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <GLFW/glfw3.h>

namespace tower
{
	namespace
	{
		glm::vec3 chipColor(CampaignNodeState state)
		{
			switch (state)
			{
			case CampaignNodeState::Available:
				return { 1.0f, 0.5f, 0.0f };
			case CampaignNodeState::Cleared:
				return { 0.2f, 0.75f, 0.55f };
			case CampaignNodeState::Skipped:
				return { 0.16f, 0.16f, 0.18f };
			case CampaignNodeState::Locked:
			default:
				return { 0.28f, 0.28f, 0.32f };
			}
		}

		bool playable(CampaignNodeState state)
		{
			return state == CampaignNodeState::Available || state == CampaignNodeState::Cleared;
		}
	}

	TowerDefenseCampaignHubEngineScene::TowerDefenseCampaignHubEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		CampaignCatalog& campaign,
		CampaignProgress& progress
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_campaign(campaign),
		_progress(progress)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	TowerDefenseCampaignHubEngineScene::~TowerDefenseCampaignHubEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	void TowerDefenseCampaignHubEngineScene::initialise(
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
			"campaign_hub_ui_sheet",
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
			{ makeMenuUiRenderpass(cameraMatrixResourceId, "campaign_hub_ui_sheet") });

		buildMenu(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void TowerDefenseCampaignHubEngineScene::buildMenu(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();

		auto& column = _layoutRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 48.0f;
		column.crossAlign = hl::ui::Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		_heading = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_heading->setText("Campaign", 96);
		_heading->color = { 1.0f, 0.5f, 0.0f };

		auto& graph = column.addChild();
		graph.kind = hl::ui::Kind::Row;
		graph.gap = 16.0f;
		graph.crossAlign = hl::ui::Align::Center;

		const auto states = evaluateCampaign(_campaign.data(), _progress);
		const auto& nodes = _campaign.nodes();
		for (std::size_t i = 0; i < nodes.size(); ++i)
		{
			if (i > 0)
			{
				auto& edgeNode = graph.addChild();
				edgeNode.intrinsicSize = glm::vec2{ 40.0f, 8.0f };
				auto edge = std::make_unique<hl::ui::Panel>(edgeNode);
				edge->color = { 0.45f, 0.45f, 0.5f };
				edge->hitTestEnabled = false;
				_edges.push_back(std::move(edge));
			}

			const auto& node = nodes[i];
			auto stateIt = states.find(node.id);
			const auto state = stateIt != states.end()
				? stateIt->second
				: CampaignNodeState::Locked;

			auto& chip = graph.addChild();
			chip.kind = hl::ui::Kind::Column;
			chip.padding = hl::ui::Edges::all(16.0f);
			chip.crossAlign = hl::ui::Align::Center;
			auto panel = std::make_unique<hl::ui::Panel>(chip);
			panel->color = chipColor(state);
			panel->hitTestEnabled = false;
			_nodePanels.push_back(std::move(panel));

			auto button = std::make_unique<hl::ui::Button>(chip.addChild(), *_typeface);
			button->setText(node.id, 48);
			button->color = { 1.0f, 1.0f, 1.0f };
			if (playable(state))
			{
				const std::string id = node.id;
				button->onClick = [this, id]()
				{
					_sceneHost.goCampaign(id);
				};
			}
			else
			{
				button->hitTestEnabled = false;
			}

			_nodeButtons.push_back(std::move(button));
		}

		_back = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_back->setText("Back", 64);
		_back->onClick = [this]() { goBack(); };
	}

	void TowerDefenseCampaignHubEngineScene::goBack()
	{
		_sceneHost.goTitle();
	}

	void TowerDefenseCampaignHubEngineScene::rebuildAndDraw(float /*delta*/)
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseCampaignHubEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		rebuildAndDraw(delta);
	}

	void TowerDefenseCampaignHubEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseCampaignHubEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}

	void TowerDefenseCampaignHubEngineScene::OnEvent(const hl::Event& event)
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
