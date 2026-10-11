#include <Views/BuildingInspectPanel.hpp>
#include <algorithm>
#include <format>

namespace drl
{

	namespace
	{
		constexpr unsigned kFontSize = 16;
		constexpr glm::vec2 kPad{ 12.0f, 12.0f };
		constexpr float kInset = 16.0f;
		constexpr float kRowGap = 6.0f;
		constexpr float kPanelWidth = 220.0f;
		constexpr std::size_t kMaxJobRows = 8;
	}

	void BuildingInspectPanel::initialise(hl::ui::ITypeface& typeface, hl::ui::Node& host)
	{
		_panel = std::make_unique<hl::ui::Panel>(host.addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;
		_panel->visible = false;

		_label = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);
		_beds = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);
		_jobs.reserve(kMaxJobRows);
		for (std::size_t i = 0; i < kMaxJobRows; ++i)
		{
			_jobs.push_back(std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface));
		}

		sync(std::nullopt);
	}

	void BuildingInspectPanel::sync(const std::optional<BuildingInspectSnapshot>& snapshot)
	{
		if (_panel == nullptr)
		{
			return;
		}

		const bool show = snapshot.has_value();
		_panel->visible = show;
		_panel->hitTestEnabled = show;
		if (!show)
		{
			return;
		}

		_label->setText(snapshot->label, kFontSize);
		if (snapshot->beds.has_value())
		{
			_beds->visible = true;
			_beds->setText(std::format("Beds: {}", *snapshot->beds), kFontSize);
		}
		else
		{
			_beds->visible = false;
			_beds->setText("", kFontSize);
		}

		const std::size_t count = std::min(_jobs.size(), snapshot->jobs.size());
		for (std::size_t i = 0; i < _jobs.size(); ++i)
		{
			if (i < count)
			{
				_jobs[i]->visible = true;
				_jobs[i]->setText(
					std::format(
						"{}: {}/{}",
						snapshot->jobs[i].label,
						snapshot->jobs[i].inUse,
						snapshot->jobs[i].total),
					kFontSize);
			}
			else
			{
				_jobs[i]->visible = false;
				_jobs[i]->setText("", kFontSize);
			}
		}
	}

	void BuildingInspectPanel::layout(glm::vec2 framebufferSize)
	{
		if (_panel == nullptr || !_panel->visible || framebufferSize.x <= 0.0f || framebufferSize.y <= 0.0f)
		{
			return;
		}

		const glm::vec2 labelSize = _label->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		const glm::vec2 bedsSize = _beds->visible
			? _beds->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f })
			: glm::vec2{ 0.0f, 0.0f };
		float jobsHeight = 0.0f;
		float maxRow = labelSize.x;
		if (_beds->visible)
		{
			maxRow = std::max(maxRow, bedsSize.x);
		}

		std::vector<glm::vec2> jobSizes(_jobs.size());
		std::size_t visibleJobs = 0;
		for (std::size_t i = 0; i < _jobs.size(); ++i)
		{
			if (!_jobs[i]->visible)
			{
				continue;
			}

			jobSizes[i] = _jobs[i]->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			maxRow = std::max(maxRow, jobSizes[i].x);
			if (visibleJobs > 0)
			{
				jobsHeight += kRowGap;
			}
			jobsHeight += jobSizes[i].y;
			++visibleJobs;
		}

		const float innerWidth = std::max(kPanelWidth - kPad.x * 2.0f, maxRow);
		float height = kPad.y * 2.0f + labelSize.y;
		if (_beds->visible)
		{
			height += kRowGap + bedsSize.y;
		}
		if (visibleJobs > 0)
		{
			height += kRowGap + jobsHeight;
		}

		const float width = innerWidth + kPad.x * 2.0f;
		_panel->node().setCenterRight({ width, height });
		_panel->node().relative = { -kInset, 0.0f };

		float y = kPad.y;
		_label->node().setTopLeft(labelSize);
		_label->node().relative = { kPad.x, y };
		_label->node().intrinsicSize.reset();
		y += labelSize.y + kRowGap;

		if (_beds->visible)
		{
			_beds->node().setTopLeft(bedsSize);
			_beds->node().relative = { kPad.x, y };
			_beds->node().intrinsicSize.reset();
			y += bedsSize.y + kRowGap;
		}

		for (std::size_t i = 0; i < _jobs.size(); ++i)
		{
			if (!_jobs[i]->visible)
			{
				continue;
			}

			_jobs[i]->node().setTopLeft(jobSizes[i]);
			_jobs[i]->node().relative = { kPad.x, y };
			_jobs[i]->node().intrinsicSize.reset();
			y += jobSizes[i].y + kRowGap;
		}
	}

}
