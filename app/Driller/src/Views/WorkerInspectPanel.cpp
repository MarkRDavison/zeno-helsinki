#include <Views/WorkerInspectPanel.hpp>
#include <Entities/Need.hpp>
#include <helsinki/Ui/Theme.hpp>
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
		constexpr float kBarWidth = 120.0f;
		constexpr float kBarHeight = 10.0f;
		constexpr float kLabelGap = 8.0f;
		constexpr float kPanelWidth = 220.0f;

		glm::vec3 bandFill(NeedBand band)
		{
			switch (band)
			{
			case NeedBand::Seek:
				return hl::ui::theme().warning;
			case NeedBand::Collapse:
				return hl::ui::theme().error;
			case NeedBand::Ok:
			default:
				return hl::ui::theme().success;
			}
		}
	}

	void WorkerInspectPanel::initialise(
		hl::ui::ITypeface& typeface,
		hl::ui::Node& host,
		const INeedPrototypeService& needPrototypes)
	{
		_panel = std::make_unique<hl::ui::Panel>(host.addChild());
		_panel->color = { 0.08f, 0.09f, 0.12f };
		_panel->opacity = 0.92f;
		_panel->hitTestEnabled = true;
		_panel->visible = false;

		_type = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);
		_activity = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);
		_job = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);

		const std::size_t needCount = needPrototypes.registeredIds().size();
		_needs.reserve(needCount);
		for (std::size_t i = 0; i < needCount; ++i)
		{
			NeedRow row{};
			row.label = std::make_unique<hl::ui::Label>(_panel->node().addChild(), typeface);
			row.bar = std::make_unique<hl::ui::ProgressBar>(_panel->node().addChild());
			_needs.push_back(std::move(row));
		}

		sync(std::nullopt);
	}

	void WorkerInspectPanel::sync(const std::optional<WorkerInspectSnapshot>& snapshot)
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

		_type->setText(snapshot->typeLabel, kFontSize);
		_activity->setText(snapshot->activity, kFontSize);
		if (snapshot->jobLabel.empty())
		{
			_job->setText("Job: none", kFontSize);
		}
		else
		{
			_job->setText(std::format("Job: {}", snapshot->jobLabel), kFontSize);
		}

		const std::size_t count = std::min(_needs.size(), snapshot->needs.size());
		for (std::size_t i = 0; i < count; ++i)
		{
			_needs[i].label->setText(snapshot->needs[i].label, kFontSize);
			_needs[i].bar->setValue(snapshot->needs[i].value / kNeedValueFull);
			_needs[i].bar->fillColor = bandFill(snapshot->needs[i].band);
		}
	}

	void WorkerInspectPanel::layout(glm::vec2 framebufferSize)
	{
		if (_panel == nullptr || !_panel->visible || framebufferSize.x <= 0.0f || framebufferSize.y <= 0.0f)
		{
			return;
		}

		const glm::vec2 typeSize = _type->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		const glm::vec2 activitySize = _activity->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		const glm::vec2 jobSize = _job->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		float needsHeight = 0.0f;
		float maxLabel = 0.0f;
		std::vector<glm::vec2> labelSizes(_needs.size());
		for (std::size_t i = 0; i < _needs.size(); ++i)
		{
			labelSizes[i] = _needs[i].label->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			maxLabel = std::max(maxLabel, labelSizes[i].x);
			needsHeight += std::max(labelSizes[i].y, kBarHeight);
			if (i + 1 < _needs.size())
			{
				needsHeight += kRowGap;
			}
		}

		const float innerWidth = std::max(
			kPanelWidth - kPad.x * 2.0f,
			std::max({ typeSize.x, activitySize.x, jobSize.x, maxLabel + kLabelGap + kBarWidth }));
		const float height = kPad.y * 2.0f + typeSize.y + kRowGap + activitySize.y + kRowGap + jobSize.y
			+ (_needs.empty() ? 0.0f : kRowGap + needsHeight);
		const float width = innerWidth + kPad.x * 2.0f;

		_panel->node().setCenterRight({ width, height });
		_panel->node().relative = { -kInset, 0.0f };

		float y = kPad.y;
		_type->node().setTopLeft(typeSize);
		_type->node().relative = { kPad.x, y };
		_type->node().intrinsicSize.reset();
		y += typeSize.y + kRowGap;

		_activity->node().setTopLeft(activitySize);
		_activity->node().relative = { kPad.x, y };
		_activity->node().intrinsicSize.reset();
		y += activitySize.y + kRowGap;

		_job->node().setTopLeft(jobSize);
		_job->node().relative = { kPad.x, y };
		_job->node().intrinsicSize.reset();
		y += jobSize.y + kRowGap;

		for (std::size_t i = 0; i < _needs.size(); ++i)
		{
			const float rowHeight = std::max(labelSizes[i].y, kBarHeight);
			_needs[i].label->node().setTopLeft(labelSizes[i]);
			_needs[i].label->node().relative = { kPad.x, y + (rowHeight - labelSizes[i].y) * 0.5f };
			_needs[i].label->node().intrinsicSize.reset();

			hl::ui::Node& bar = _needs[i].bar->node();
			bar.anchorMin = { 0.0f, 0.0f };
			bar.anchorMax = { 1.0f, 0.0f };
			bar.offset.left = kPad.x + maxLabel + kLabelGap;
			bar.offset.right = -kPad.x;
			bar.offset.top = 0.0f;
			bar.offset.bottom = kBarHeight;
			bar.relative = { 0.0f, y + (rowHeight - kBarHeight) * 0.5f };
			bar.intrinsicSize.reset();
			y += rowHeight + kRowGap;
		}
	}

}
