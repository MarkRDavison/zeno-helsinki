#include "Scenes/TowerDefenseResearchEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <Services/GraphChip.hpp>
#include <Services/GraphEvaluator.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace tower
{
	TowerDefenseResearchEngineScene::TowerDefenseResearchEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		ResearchCatalog& research,
		ProfileService& profile
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_research(research),
		_profile(profile)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	TowerDefenseResearchEngineScene::~TowerDefenseResearchEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	void TowerDefenseResearchEngineScene::initialise(
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
			"research_ui_sheet",
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
			{ makeMenuUiRenderpass(cameraMatrixResourceId, "research_ui_sheet") });

		buildMenu(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void TowerDefenseResearchEngineScene::buildMenu(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();

		auto& column = _layoutRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 32.0f;
		column.crossAlign = hl::ui::Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		_heading = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_heading->setText("Research", 96);
		_heading->color = { 1.0f, 0.5f, 0.0f };

		_points = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_points->color = { 1.0f, 1.0f, 1.0f };

		auto& graph = column.addChild();
		graph.kind = hl::ui::Kind::Row;
		graph.gap = 16.0f;
		graph.crossAlign = hl::ui::Align::Center;

		const auto graphNodes = researchGraphNodes(_research.data());
		const auto depths = graphNodeDepths(graphNodes);
		int maxDepth = 0;
		for (const auto& [id, depth] : depths)
		{
			maxDepth = std::max(maxDepth, depth);
		}

		std::vector<std::vector<std::size_t>> columns(static_cast<std::size_t>(maxDepth) + 1);
		const auto& nodes = _research.nodes();
		for (std::size_t i = 0; i < nodes.size(); ++i)
		{
			const auto it = depths.find(nodes[i].id);
			const int depth = it != depths.end() ? it->second : 0;
			columns[static_cast<std::size_t>(depth)].push_back(i);
		}

		for (std::size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex)
		{
			if (columnIndex > 0)
			{
				auto& edgeNode = graph.addChild();
				edgeNode.intrinsicSize = glm::vec2{ 40.0f, 8.0f };
				auto edge = std::make_unique<hl::ui::Panel>(edgeNode);
				edge->color = { 0.45f, 0.45f, 0.5f };
				edge->hitTestEnabled = false;
				_edges.push_back(std::move(edge));
			}

			auto& stack = graph.addChild();
			stack.kind = hl::ui::Kind::Column;
			stack.gap = 16.0f;
			stack.crossAlign = hl::ui::Align::Center;

			for (const auto nodeIndex : columns[columnIndex])
			{
				const auto& node = nodes[nodeIndex];
				auto& chip = stack.addChild();
				chip.kind = hl::ui::Kind::Column;
				chip.padding = hl::ui::Edges::all(16.0f);
				chip.crossAlign = hl::ui::Align::Center;
				auto panel = std::make_unique<hl::ui::Panel>(chip);
				panel->hitTestEnabled = false;
				_nodePanels.push_back(std::move(panel));

				auto button = std::make_unique<hl::ui::Button>(chip.addChild(), *_typeface);
				button->color = { 1.0f, 1.0f, 1.0f };
				const std::string id = node.id;
				button->onClick = [this, id]() { tryBuy(id); };
				_nodeIds.push_back(id);
				_nodeButtons.push_back(std::move(button));
			}
		}

		_back = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_back->setText("Back", 64);
		_back->onClick = [this]() { goBack(); };

		refreshChrome();
	}

	void TowerDefenseResearchEngineScene::refreshChrome()
	{
		_points->setText("Points: " + std::to_string(_profile.profile().points), 48);
		const auto states = evaluateGraph(
			researchGraphNodes(_research.data()),
			_profile.profile().researched);
		for (std::size_t i = 0; i < _nodeIds.size(); ++i)
		{
			const auto* node = _research.find(_nodeIds[i]);
			if (node == nullptr)
			{
				continue;
			}

			const auto it = states.find(node->id);
			const auto state = it != states.end() ? it->second : GraphNodeState::Locked;
			_nodePanels[i]->color = graphChipColor(state);

			std::string label = node->label;
			if (node->cost > 0)
			{
				label += " (" + std::to_string(node->cost) + ")";
			}

			_nodeButtons[i]->setText(label, 48);
			const bool affordable = _profile.profile().points >= node->cost;
			_nodeButtons[i]->hitTestEnabled =
				state == GraphNodeState::Available && affordable;
		}
	}

	void TowerDefenseResearchEngineScene::tryBuy(const std::string& nodeId)
	{
		if (_profile.tryResearch(nodeId, _research.data()))
		{
			refreshChrome();
		}
	}

	void TowerDefenseResearchEngineScene::goBack()
	{
		_sceneHost.goCampaignHub();
	}

	void TowerDefenseResearchEngineScene::rebuildAndDraw(float /*delta*/)
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseResearchEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		rebuildAndDraw(delta);
	}

	void TowerDefenseResearchEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseResearchEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}

	void TowerDefenseResearchEngineScene::OnEvent(const hl::Event& event)
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
