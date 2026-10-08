#pragma once

#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/Engine/Engine.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/System/Events/EventListener.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Dropdown.hpp>
#include <helsinki/Ui/Checkbox.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/RadioGroup.hpp>
#include <helsinki/Ui/Slider.hpp>
#include <helsinki/Ui/TextField.hpp>
#include <helsinki/Ui/Toggle.hpp>
#include <memory>
#include <vector>

namespace hl
{
	class FontResource;
}

namespace ui
{
	class UserInterfaceStartEngineScene : public hl::EngineScene, public hl::EventListener
	{
	public:
		UserInterfaceStartEngineScene(
			hl::Engine& engine,
			const hl::EngineConfiguration& engineConfig);
		~UserInterfaceStartEngineScene();

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
		void buildLayoutTree();
		void rebuildAndDraw();

		const hl::EngineConfiguration& _engineConfig;
		hl::UiBatch _uiBatch;
		std::unique_ptr<hl::ui::ITypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _layoutRoot;
		std::vector<std::unique_ptr<hl::ui::Widget>> _widgets;
		std::unique_ptr<hl::ui::Slider> _slider;
		std::unique_ptr<hl::ui::Checkbox> _checkbox;
		std::unique_ptr<hl::ui::Toggle> _toggle;
		std::unique_ptr<hl::ui::Dropdown> _dropdown;
		std::unique_ptr<hl::ui::RadioGroup> _radioVertical;
		std::unique_ptr<hl::ui::RadioGroup> _radioHorizontal;
		std::unique_ptr<hl::ui::Label> _fieldLabel;
		std::unique_ptr<hl::ui::TextField> _textField;
		std::unique_ptr<hl::ui::Button> _actionButton;
		hl::ui::Panel* _clipHitRow = nullptr;
		bool _clipHitOn = false;
	};
}
