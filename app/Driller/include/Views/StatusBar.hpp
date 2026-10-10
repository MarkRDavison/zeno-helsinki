#pragma once

#include <Views/StatusBarChips.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Image.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Tooltip.hpp>
#include <memory>
#include <vector>

namespace drl
{

	class StatusBar
	{
	public:
		void initialise(
			hl::ui::ITypeface& typeface,
			hl::ui::Node& host,
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);
		void syncValues(
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);
		void layout(glm::vec2 framebufferSize);
		void tickOverlays(float dt);

	private:
		struct ChipWidgets
		{
			StatusChip spec;
			std::unique_ptr<hl::ui::Panel> panel;
			std::unique_ptr<hl::ui::Image> icon;
			std::unique_ptr<hl::ui::Label> value;
		};

		hl::ui::ITypeface* _typeface{ nullptr };
		std::unique_ptr<hl::ui::Panel> _panel;
		std::vector<ChipWidgets> _chips;
		std::unique_ptr<hl::ui::Tooltip> _tooltip;
	};

}
