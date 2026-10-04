#include <helsinki/Ui/Slider.hpp>

#include <algorithm>

namespace hl::ui
{
	Slider::Slider(Node& node) :
		Widget(node)
	{
		node.intrinsicSize = glm::vec2{ 280.0f, 28.0f };
	}

	void Slider::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ 280.0f, 28.0f };
		}
	}

	void Slider::setValue(float value)
	{
		_value = std::clamp(value, 0.0f, 1.0f);
	}

	void Slider::applyPointer(const Pointer& pointer)
	{
		const auto& box = node().world;
		const float width = std::max(box.size.x, 1.0f);
		setValue((pointer.position.x - box.pos.x) / width);
		if (onChanged)
		{
			onChanged(_value);
		}
	}

	EventResult Slider::handle(const Pointer& pointer)
	{
		const bool inside = node().world.contains(pointer.position);

		if (pointer.primaryDown && (inside || hasPointerCapture()))
		{
			capturePointer();
			applyPointer(pointer);
			return EventResult::Consume;
		}

		if (hasPointerCapture() && !pointer.primaryDown)
		{
			releasePointer();
			return EventResult::Consume;
		}

		return inside ? EventResult::Consume : EventResult::Ignore;
	}

	void Slider::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		const float trackH = std::max(8.0f, box.size.y * 0.28f);
		const float trackY = box.pos.y + (box.size.y - trackH) * 0.5f;
		paint.fill(Box{ box.pos.x, trackY, box.size.x, trackH }, trackColor);

		const float filled = box.size.x * _value;
		if (filled > 0.0f)
		{
			paint.fill(Box{ box.pos.x, trackY, filled, trackH }, fillColor);
		}

		const float thumb = std::min(box.size.y, 22.0f);
		const float thumbX = box.pos.x + filled - thumb * 0.5f;
		const float thumbY = box.pos.y + (box.size.y - thumb) * 0.5f;
		paint.fill(Box{ thumbX, thumbY, thumb, thumb }, thumbColor);
	}
}
