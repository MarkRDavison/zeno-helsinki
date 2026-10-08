#include <helsinki/Ui/Button.hpp>

namespace hl::ui
{
	Button::Button(Node& node, const ITypeface& typeface) :
		Label(node, typeface)
	{
		hitTestEnabled = true;
		focusable = true;
	}

	void Button::prepare()
	{
		Label::prepare();
		setDrawColor(_hovered || hasKeyboardFocus() ? hoverColor : color);
	}

	EventResult Button::handle(const Pointer& pointer)
	{
		_hovered = node().world.contains(pointer.position);
		if (_hovered && pointer.primaryReleased)
		{
			setFocused(true);
			if (onClick)
			{
				onClick();
			}
		}

		setDrawColor(_hovered || hasKeyboardFocus() ? hoverColor : color);
		return _hovered ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult Button::handleKey(TextKey key)
	{
		if (key != TextKey::Enter && key != TextKey::Space)
		{
			return EventResult::Ignore;
		}

		if (onClick)
		{
			onClick();
		}

		return EventResult::Consume;
	}

	void Button::paint(IPaint& paint) const
	{
		Label::paint(paint);
	}
}
