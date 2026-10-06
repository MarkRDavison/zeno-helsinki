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
#include <Services/CampaignCatalog.hpp>
#include <Services/CampaignTypes.hpp>
#include <memory>
#include <vector>

namespace hl
{
	class FontResource;
}

namespace tower
{
	class SceneHost;

	class TowerDefenseCampaignHubEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		TowerDefenseCampaignHubEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost,
			CampaignCatalog& campaign,
			CampaignProgress& progress);
		~TowerDefenseCampaignHubEngineScene();

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
		void rebuildAndDraw(float delta);
		void goBack();

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		CampaignCatalog& _campaign;
		CampaignProgress& _progress;
		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Label> _heading;
		std::vector<std::unique_ptr<hl::ui::Panel>> _nodePanels;
		std::vector<std::unique_ptr<hl::ui::Panel>> _edges;
		std::vector<std::unique_ptr<hl::ui::Button>> _nodeButtons;
		std::unique_ptr<hl::ui::Button> _back;
	};
}
