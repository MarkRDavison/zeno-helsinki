#include <helsinki/Ui/Checkbox.hpp>

#include <algorithm>

namespace hl::ui
{
	Checkbox::Checkbox(Node& node) :
		Widget(node)
	{
		node.intrinsicSize = glm::vec2{ 32.0f, 32.0f };
	}

	void Checkbox::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ 32.0f, 32.0f };
		}
	}

	void Checkbox::setChecked(bool checked)
	{
		_checked = checked;
	}

	EventResult Checkbox::handle(const Pointer& pointer)
	{
		const bool inside = node().world.contains(pointer.position);
		if (inside && pointer.primaryReleased)
		{
			setChecked(!_checked);
			if (onChanged)
			{
				onChanged(_checked);
			}
		}

		return inside ? EventResult::Consume : EventResult::Ignore;
	}

	void Checkbox::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		paint.fill(box, boxColor);

		if (_checked)
		{
			const float inset = std::min(box.size.x, box.size.y) * 0.22f;
			paint.fill(
				Box{
					box.pos.x + inset,
					box.pos.y + inset,
					box.size.x - inset * 2.0f,
					box.size.y - inset * 2.0f
				},
				checkColor);
		}
	}
}
