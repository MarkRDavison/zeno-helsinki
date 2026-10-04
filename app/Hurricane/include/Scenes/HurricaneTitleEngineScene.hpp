#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <Ui/UiLayoutAdapters.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <memory>

namespace hl
{
	class FontResource;
}

namespace hur
{
	class SceneHost;

	class HurricaneTitleEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		HurricaneTitleEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost);
		~HurricaneTitleEngineScene();

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

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Label> _title;
		std::unique_ptr<hl::ui::Button> _start;
		std::unique_ptr<hl::ui::Button> _settings;
		std::unique_ptr<hl::ui::Button> _quit;
	};
}
