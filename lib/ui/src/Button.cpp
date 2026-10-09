#include <helsinki/Ui/Button.hpp>

#include <algorithm>

namespace hl::ui
{
	Button::Button(Node& node, const ITypeface& typeface) :
		Label(node, typeface)
	{
		hitTestEnabled = true;
		focusable = true;
	}

	float Button::resolvedBorderWidth() const
	{
		if (variant != ButtonVariant::Default)
		{
			return resolve(borderWidth, 0.0f);
		}

		return resolve(borderWidth, std::max(theme().borderWidth, 1.0f));
	}

	void Button::applyDrawColor()
	{
		setDrawColor(_hovered ? resolvedHoverColor() : resolvedColor());
	}

	void Button::prepare()
	{
		_hovered = false;
		opacity = enabled ? 1.0f : 0.45f;
		Label::prepare();
		applyDrawColor();
		if (!chrome())
		{
			return;
		}

		const float pad = theme().padding;
		const float width = resolvedBorderWidth();
		node().intrinsicSize = textLayoutSize() + glm::vec2{ (pad + width) * 2.0f };
	}

	EventResult Button::handle(const Pointer& pointer)
	{
		_hovered = node().world.contains(pointer.position);
		if (enabled && _hovered && pointer.primaryReleased)
		{
			setFocused(true);
			if (onClick)
			{
				onClick();
			}
		}

		applyDrawColor();
		return _hovered ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult Button::handleKey(TextKey key)
	{
		if (!enabled)
		{
			return EventResult::Ignore;
		}

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
		const auto& box = node().world;
		glm::vec2 origin = box.pos;
		if (chrome())
		{
			const float width = resolvedBorderWidth();
			const glm::vec3 fill = _hovered ? theme().hover : resolvedFillColor();
			const glm::vec3 edge =
				variant == ButtonVariant::Filled ? fill : resolvedBorderColor();
			if (width > 0.0f)
			{
				paint.fill(box, glm::vec4{ edge, opacity });
				const float inset = width * 2.0f;
				if (box.size.x > inset && box.size.y > inset)
				{
					paint.fill(
						Box{
							box.pos.x + width,
							box.pos.y + width,
							box.size.x - inset,
							box.size.y - inset
						},
						glm::vec4{ fill, opacity });
				}
			}
			else
			{
				paint.fill(box, glm::vec4{ fill, opacity });
			}

			origin.x += (box.size.x - textLayoutSize().x) * 0.5f;
			origin.y += (box.size.y - textLayoutSize().y) * 0.5f;
		}

		paintGlyphs(paint, origin);
	}
}
