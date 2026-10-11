#pragma once

#include <Views/BuildingInspect.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace drl
{

	class BuildingInspectPanel
	{
	public:
		void initialise(hl::ui::ITypeface& typeface, hl::ui::Node& host);
		void sync(const std::optional<BuildingInspectSnapshot>& snapshot);
		void layout(glm::vec2 framebufferSize);

	private:
		std::unique_ptr<hl::ui::Panel> _panel;
		std::unique_ptr<hl::ui::Label> _label;
		std::unique_ptr<hl::ui::Label> _beds;
		std::vector<std::unique_ptr<hl::ui::Label>> _jobs;
	};

}
