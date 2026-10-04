#include <helsinki/Ui/Button.hpp>

namespace hl::ui
{
	Button::Button(Node& node, const ITypeface& typeface) :
		Label(node, typeface)
	{
		hitTestEnabled = true;
	}

	EventResult Button::handle(const Pointer& pointer)
	{
		_hovered = node().world.contains(pointer.position);
		setDrawColor(_hovered ? hoverColor : color);

		if (_hovered && pointer.primaryReleased && onClick)
		{
			onClick();
		}

		return _hovered ? EventResult::Consume : EventResult::Ignore;
	}

	void Button::paint(IPaint& paint) const
	{
		Label::paint(paint);
	}
}
