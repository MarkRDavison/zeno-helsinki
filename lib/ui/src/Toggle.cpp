#include <helsinki/Ui/Toggle.hpp>

#include <algorithm>

namespace hl::ui
{
	Toggle::Toggle(Node& node) :
		Widget(node)
	{
		focusable = true;
	}

	void Toggle::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ theme().controlHeight * 2.0f, theme().controlHeight };
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
			setFocused(true);
			setOn(!_on);
			if (onChanged)
			{
				onChanged(_on);
			}
		}

		return inside ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult Toggle::handleKey(TextKey key)
	{
		if (key != TextKey::Enter && key != TextKey::Space)
		{
			return EventResult::Ignore;
		}

		setOn(!_on);
		if (onChanged)
		{
			onChanged(_on);
		}

		return EventResult::Consume;
	}

	void Toggle::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		const glm::vec3 track = _on ? resolvedOnColor() : resolvedOffColor();
		const glm::vec3 fill = hasKeyboardFocus()
			? glm::vec3{
				std::min(1.0f, track.x + 0.18f),
				std::min(1.0f, track.y + 0.18f),
				std::min(1.0f, track.z + 0.18f)
			}
			: track;
		paint.fill(box, fill);

		const float pad = std::min(box.size.x, box.size.y) * 0.12f;
		const float knob = box.size.y - pad * 2.0f;
		const float knobX = _on
			? box.pos.x + box.size.x - pad - knob
			: box.pos.x + pad;
		paint.fill(Box{ knobX, box.pos.y + pad, knob, knob }, resolvedKnobColor());
	}
}
