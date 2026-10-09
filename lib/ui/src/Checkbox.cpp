#include <helsinki/Ui/Checkbox.hpp>

#include <algorithm>

namespace hl::ui
{
	Checkbox::Checkbox(Node& node) :
		Widget(node)
	{
		focusable = true;
	}

	void Checkbox::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			const float h = theme().controlHeight;
			node().intrinsicSize = glm::vec2{ h, h };
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
			setFocused(true);
			setChecked(!_checked);
			if (onChanged)
			{
				onChanged(_checked);
			}
		}

		return inside ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult Checkbox::handleKey(TextKey key)
	{
		if (key != TextKey::Enter && key != TextKey::Space)
		{
			return EventResult::Ignore;
		}

		setChecked(!_checked);
		if (onChanged)
		{
			onChanged(_checked);
		}

		return EventResult::Consume;
	}

	void Checkbox::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		const glm::vec3 well = resolvedBoxColor();
		const glm::vec3 fill = hasKeyboardFocus()
			? glm::vec3{
				std::min(1.0f, well.x + 0.18f),
				std::min(1.0f, well.y + 0.18f),
				std::min(1.0f, well.z + 0.18f)
			}
			: well;
		paint.fill(box, fill);

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
				resolvedCheckColor());
		}
	}
}
