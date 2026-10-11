#pragma once

#include <Services/NeedPrototypeService.hpp>
#include <Views/WorkerInspect.hpp>
#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/ProgressBar.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace drl
{

	class WorkerInspectPanel
	{
	public:
		void initialise(
			hl::ui::ITypeface& typeface,
			hl::ui::Node& host,
			const INeedPrototypeService& needPrototypes);
		void sync(const std::optional<WorkerInspectSnapshot>& snapshot);
		void layout(glm::vec2 framebufferSize);

	private:
		struct NeedRow
		{
			std::unique_ptr<hl::ui::Label> label;
			std::unique_ptr<hl::ui::ProgressBar> bar;
		};

		std::unique_ptr<hl::ui::Panel> _panel;
		std::unique_ptr<hl::ui::Label> _type;
		std::unique_ptr<hl::ui::Label> _activity;
		std::unique_ptr<hl::ui::Label> _job;
		std::vector<NeedRow> _needs;
	};

}
