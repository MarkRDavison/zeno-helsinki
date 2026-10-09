#pragma once

#include <Views/StatusBarChips.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Image.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <helsinki/Ui/Tooltip.hpp>
#include <memory>
#include <vector>

namespace hl
{
	class FontResource;
}

namespace drl
{

	class StatusBar
	{
	public:
		void initialise(
			hl::FontResource* font,
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);
		void tick(
			hl::UiBatch& batch,
			glm::vec2 framebufferSize,
			const hl::ui::Pointer& pointer,
			float dt,
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);
		bool hits(glm::vec2 position) const;

	private:
		struct ChipWidgets
		{
			StatusChip spec;
			std::unique_ptr<hl::ui::Panel> panel;
			std::unique_ptr<hl::ui::Image> icon;
			std::unique_ptr<hl::ui::Label> value;
		};

		void syncValues(
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);

		std::unique_ptr<hl::ui::ITypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _root;
		std::unique_ptr<hl::ui::Panel> _panel;
		std::vector<ChipWidgets> _chips;
		std::unique_ptr<hl::ui::Tooltip> _tooltip;
	};

}
