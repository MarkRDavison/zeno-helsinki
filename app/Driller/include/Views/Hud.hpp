#pragma once

#include <Views/BuildBar.hpp>
#include <Views/StatusBar.hpp>
#include <helsinki/Engine/Ui/UiBatch.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <helsinki/Ui/Snackbar.hpp>
#include <memory>

namespace hl
{
	class FontResource;
}

namespace drl
{

	class Hud
	{
	public:
		void initialise(
			hl::FontResource* font,
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades,
			const BuildingPrototypeService& buildingPrototypes,
			IUiService& ui);
		void tick(
			hl::UiBatch& batch,
			glm::vec2 framebufferSize,
			const hl::ui::Pointer& pointer,
			float dt,
			const IEconomyResourceService& economy,
			const IUpgradeService& upgrades);
		bool hits(glm::vec2 position) const;
		void show(hl::ui::SnackbarItem item);

	private:
		std::unique_ptr<hl::ui::ITypeface> _typeface;
		std::unique_ptr<hl::ui::Node> _root;
		StatusBar _statusBar;
		BuildBar _buildBar;
		std::unique_ptr<hl::ui::SnackbarHost> _snackbar;
	};

}
