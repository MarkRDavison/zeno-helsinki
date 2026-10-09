#include <helsinki/Ui/ProgressBar.hpp>

#include <algorithm>

namespace hl::ui
{
	ProgressBar::ProgressBar(Node& node) :
		Widget(node)
	{
		hitTestEnabled = false;
		focusable = false;
	}

	void ProgressBar::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ theme().controlWidth, theme().controlHeight * 0.5f };
		}
	}

	void ProgressBar::setValue(float value)
	{
		_value = std::clamp(value, 0.0f, 1.0f);
	}

	void ProgressBar::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		paint.fill(box, resolvedTrackColor());

		const float filled = box.size.x * _value;
		if (filled > 0.0f)
		{
			paint.fill(Box{ box.pos.x, box.pos.y, filled, box.size.y }, resolvedFillColor());
		}
	}
}
