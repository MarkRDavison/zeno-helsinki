#pragma once

#include <Services/BuildingPrototypeService.hpp>
#include <Services/EconomyResourceService.hpp>
#include <Services/UiService.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <memory>
#include <vector>

namespace hl
{
	class FontResource;
}

namespace drl
{

	class BuildBar
	{
	public:
		void initialise(
			hl::FontResource* font,
			const BuildingPrototypeService& buildingPrototypes,
			IUiService& ui);
		void syncEnabled(const IEconomyResourceService& economy);
		void tick(hl::UiBatch& batch, glm::vec2 framebufferSize, const hl::ui::Pointer& pointer);
		bool hits(glm::vec2 position) const;

	private:
		struct Slot
		{
			std::unique_ptr<hl::ui::Button> button;
			long long cost{ 0 };
		};

		IUiService* _ui{ nullptr };
		std::unique_ptr<hl::ui::ITypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _root;
		std::unique_ptr<hl::ui::Panel> _panel;
		std::vector<Slot> _slots;
	};

}
