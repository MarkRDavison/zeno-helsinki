#include <helsinki/Ui/Toggle.hpp>

#include <algorithm>

namespace hl::ui
{
	Toggle::Toggle(Node& node) :
		Widget(node)
	{
		node.intrinsicSize = glm::vec2{ 64.0f, 32.0f };
	}

	void Toggle::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ 64.0f, 32.0f };
		}
	}

	void Toggle::setOn(bool on)
	{
		_on = on;
	}

	EventResult Toggle::handle(const Pointer& pointer)
	{
		const bool inside = node().world.contains(pointer.position);
		if (inside && pointer.primaryReleased)
		{
			setOn(!_on);
			if (onChanged)
			{
				onChanged(_on);
			}
		}

		return inside ? EventResult::Consume : EventResult::Ignore;
	}

	void Toggle::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		paint.fill(box, _on ? onColor : offColor);

		const float pad = std::min(box.size.x, box.size.y) * 0.12f;
		const float knob = box.size.y - pad * 2.0f;
		const float knobX = _on
			? box.pos.x + box.size.x - pad - knob
			: box.pos.x + pad;
		paint.fill(Box{ knobX, box.pos.y + pad, knob, knob }, knobColor);
	}
}
