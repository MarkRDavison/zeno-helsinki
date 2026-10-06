#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <Ui/UiLayoutAdapters.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <Services/ProfileService.hpp>
#include <Services/ResearchCatalog.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hl
{
	class FontResource;
}

namespace tower
{
	class SceneHost;

	class TowerDefenseResearchEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		TowerDefenseResearchEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost,
			ResearchCatalog& research,
			ProfileService& profile);
		~TowerDefenseResearchEngineScene();

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
		void buildMenu(hl::FontResource* font);
		void refreshChrome();
		void rebuildAndDraw(float delta);
		void goBack();
		void tryBuy(const std::string& nodeId);

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		ResearchCatalog& _research;
		ProfileService& _profile;
		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Label> _heading;
		std::unique_ptr<hl::ui::Label> _points;
		std::vector<std::string> _nodeIds;
		std::vector<std::unique_ptr<hl::ui::Panel>> _nodePanels;
		std::vector<std::unique_ptr<hl::ui::Panel>> _edges;
		std::vector<std::unique_ptr<hl::ui::Button>> _nodeButtons;
		std::unique_ptr<hl::ui::Button> _back;
	};
}
