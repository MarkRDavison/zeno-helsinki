#pragma once

#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/UiService.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <memory>
#include <vector>

namespace drl
{

	class BuildBar
	{
	public:
		void initialise(
			hl::ui::ITypeface& typeface,
			hl::ui::Node& host,
			const BuildingPrototypeService& buildingPrototypes,
			IUiService& ui);
		void syncEnabled(const IEconomyResourceService& economy);
		void layout();

	private:
		struct Slot
		{
			std::unique_ptr<hl::ui::Button> button;
			long long cost{ 0 };
		};

		IUiService* _ui{ nullptr };
		hl::ui::ITypeface* _typeface{ nullptr };
		std::unique_ptr<hl::ui::Panel> _panel;
		std::vector<Slot> _slots;
	};

}
