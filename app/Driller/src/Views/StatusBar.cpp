#include <Views/StatusBar.hpp>
#include <algorithm>

namespace drl
{

	namespace
	{
		constexpr unsigned kBarFontSize = 18;
		constexpr glm::vec2 kIconSize{ 24.0f, 24.0f };
		constexpr float kChipGap = 8.0f;
		constexpr float kIconTextGap = 6.0f;
		constexpr glm::vec2 kBarPad{ 16.0f, 8.0f };

		glm::vec3 placeholderTint(std::size_t index)
		{
			constexpr glm::vec3 tints[] = {
				{ 0.72f, 0.48f, 0.28f },
				{ 0.86f, 0.72f, 0.22f },
				{ 0.32f, 0.72f, 0.42f },
				{ 0.38f, 0.58f, 0.86f },
				{ 0.72f, 0.42f, 0.68f }
			};
			return tints[index % (sizeof(tints) / sizeof(tints[0]))];
		}
	}

	void StatusBar::initialise(
		hl::ui::ITypeface& typeface,
		hl::ui::Node& host,
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		_typeface = &typeface;

		_panel = std::make_unique<hl::ui::Panel>(host.addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;

		const auto specs = collectStatusBarChips(economy, upgrades);
		for (std::size_t i = 0; i < specs.size(); ++i)
		{
			ChipWidgets chip;
			chip.spec = specs[i];
			chip.panel = std::make_unique<hl::ui::Panel>(_panel->node().addChild());
			chip.panel->opacity = 0.0f;
			chip.panel->hitTestEnabled = true;

			chip.icon = std::make_unique<hl::ui::Image>(chip.panel->node().addChild());
			chip.icon->size = kIconSize;
			chip.icon->color = placeholderTint(i);

			chip.value = std::make_unique<hl::ui::Label>(chip.panel->node().addChild(), typeface);
			chip.value->setText("0", kBarFontSize);

			_chips.push_back(std::move(chip));
		}

		_tooltip = std::make_unique<hl::ui::Tooltip>(host.addChild(), typeface);
		_tooltip->delay = 0.0f;
		syncValues(economy, upgrades);
	}

	void StatusBar::syncValues(
		const IEconomyResourceService& economy,
		const IUpgradeService& upgrades)
	{
		for (ChipWidgets& chip : _chips)
		{
			const std::string value = formatStatusValue(chip.spec, economy, upgrades);
			chip.value->setText(value, kBarFontSize);
			chip.panel->tooltip = formatStatusTooltip(chip.spec.label, value, chip.spec.description);
		}
	}

	void StatusBar::layout(glm::vec2 framebufferSize)
	{
		if (_panel == nullptr)
		{
			return;
		}

		float chipsHeight = kIconSize.y;
		std::vector<glm::vec2> chipSizes(_chips.size());
		for (std::size_t i = 0; i < _chips.size(); ++i)
		{
			const glm::vec2 textSize = _chips[i].value->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chipSizes[i] = {
				kIconSize.x + kIconTextGap + textSize.x,
				std::max(kIconSize.y, textSize.y)
			};
			chipsHeight = std::max(chipsHeight, chipSizes[i].y);
		}

		const float barHeight = chipsHeight + kBarPad.y * 2.0f;
		_panel->node().setTopLeft({ framebufferSize.x, barHeight });
		_panel->node().relative = { 0.0f, 0.0f };

		float chipX = kBarPad.x;
		for (std::size_t i = 0; i < _chips.size(); ++i)
		{
			ChipWidgets& chip = _chips[i];
			chip.panel->node().setTopLeft(chipSizes[i]);
			chip.panel->node().relative = { chipX, kBarPad.y };
			chip.panel->node().intrinsicSize.reset();

			chip.icon->node().setCenterLeft(kIconSize);
			chip.icon->node().relative = { 0.0f, 0.0f };
			chip.icon->node().intrinsicSize.reset();

			const glm::vec2 textSize = chip.value->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chip.value->node().setCenterLeft(textSize);
			chip.value->node().relative = { kIconSize.x + kIconTextGap, 0.0f };
			chip.value->node().intrinsicSize.reset();

			chipX += chipSizes[i].x + kChipGap;
		}
	}

	void StatusBar::tickOverlays(float dt)
	{
		if (_tooltip != nullptr)
		{
			_tooltip->tick(dt);
		}
	}

}
