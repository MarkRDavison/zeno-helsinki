#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Checkbox.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Slider.hpp>
#include <helsinki/Ui/Toggle.hpp>
#include <Ui/UiLayoutAdapters.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/Audio/Audio.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <memory>

namespace hl
{
	class FontResource;
}

namespace tower
{
	class SceneHost;

	class TowerDefenseSettingsEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		TowerDefenseSettingsEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig,
			SceneHost& sceneHost,
			hl::audio::Audio& audio);
		~TowerDefenseSettingsEngineScene();

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
		void buildUi(hl::FontResource* font);
		void rebuildAndDraw(float delta);
		void goBack();

		SceneHost& _sceneHost;
		const hl::EngineConfiguration& _engineConfig;
		hl::audio::Audio& _audio;
		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::unique_ptr<FontTypeface> _typeface;
		std::unique_ptr<hl::ui::Panel> _backdrop;
		std::unique_ptr<hl::ui::Panel> _card;
		std::unique_ptr<hl::ui::Label> _heading;
		std::unique_ptr<hl::ui::Label> _volumeLabel;
		std::unique_ptr<hl::ui::Label> _volumePercent;
		std::unique_ptr<hl::ui::Slider> _volume;
		std::unique_ptr<hl::ui::Label> _musicLabel;
		std::unique_ptr<hl::ui::Checkbox> _music;
		std::unique_ptr<hl::ui::Label> _fullscreenLabel;
		std::unique_ptr<hl::ui::Toggle> _fullscreen;
		std::unique_ptr<hl::ui::Label> _vsyncLabel;
		std::unique_ptr<hl::ui::Toggle> _vsync;
		std::unique_ptr<hl::ui::Button> _back;
	};
}
