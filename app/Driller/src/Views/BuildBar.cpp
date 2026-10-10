#include <Views/BuildBar.hpp>
#include <Services/PrototypeService.hpp>
#include <algorithm>
#include <format>

namespace drl
{

	namespace
	{
		constexpr unsigned kBarFontSize = 18;
	}

	void BuildBar::initialise(
		hl::ui::ITypeface& typeface,
		hl::ui::Node& host,
		const BuildingPrototypeService& buildingPrototypes,
		IUiService& ui)
	{
		_ui = &ui;
		_typeface = &typeface;

		_panel = std::make_unique<hl::ui::Panel>(host.addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;

		for (const std::string& name : buildingPrototypes.registeredNames())
		{
			const BuildingPrototype& prototype =
				buildingPrototypes.getPrototype(prototypeIdFromName(name));
			auto button = std::make_unique<hl::ui::Button>(
				_panel->node().addChild(),
				typeface);
			button->setText(std::format("{} ({})", prototype.label, prototype.cost), kBarFontSize);
			button->onClick = [this, name]()
			{
				if (_ui != nullptr)
				{
					_ui->selectBuilding(name);
				}
			};

			Slot slot{};
			slot.cost = prototype.cost;
			slot.button = std::move(button);
			_slots.push_back(std::move(slot));
		}
	}

	void BuildBar::syncEnabled(const IEconomyResourceService& economy)
	{
		for (Slot& slot : _slots)
		{
			if (slot.button != nullptr)
			{
				slot.button->enabled = economy.canAfford(ResourceMoney, slot.cost);
			}
		}
	}

	void BuildBar::layout()
	{
		if (_panel == nullptr)
		{
			return;
		}

		constexpr glm::vec2 barPad{ 16.0f, 8.0f };
		constexpr float chipGap = 8.0f;
		float chipsWidth = 0.0f;
		float chipsHeight = 0.0f;
		std::vector<glm::vec2> chipSizes(_slots.size());
		for (std::size_t i = 0; i < _slots.size(); ++i)
		{
			chipSizes[i] = _slots[i].button->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chipsWidth += chipSizes[i].x;
			chipsHeight = std::max(chipsHeight, chipSizes[i].y);
		}
		if (!_slots.empty())
		{
			chipsWidth += chipGap * static_cast<float>(_slots.size() - 1);
		}

		_panel->node().setBottomCenter(
			{ chipsWidth + barPad.x * 2.0f, chipsHeight + barPad.y * 2.0f });
		_panel->node().relative = { 0.0f, 0.0f };

		float chipX = barPad.x;
		for (std::size_t i = 0; i < _slots.size(); ++i)
		{
			_slots[i].button->node().setTopLeft(chipSizes[i]);
			_slots[i].button->node().relative = { chipX, barPad.y };
			_slots[i].button->node().intrinsicSize.reset();
			chipX += chipSizes[i].x + chipGap;
		}
	}

}
