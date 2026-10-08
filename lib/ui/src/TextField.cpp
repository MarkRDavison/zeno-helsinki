#include <helsinki/Ui/TextField.hpp>

#include <algorithm>

namespace hl::ui
{
	TextField::TextField(Node& node, const ITypeface& typeface) :
		Widget(node),
		_typeface(&typeface)
	{
		hitTestEnabled = true;
		focusable = true;
		node.clip = true;
		node.padding = Edges::all(_padding);
		node.intrinsicSize = glm::vec2{ 280.0f, 36.0f };
	}

	void TextField::setText(std::string text)
	{
		_text = std::move(text);
		_caret = std::min(_caret, _text.size());
		notifyChanged();
	}

	EventResult TextField::handle(const Pointer& pointer)
	{
		const bool inside = node().world.contains(pointer.position);
		if (pointer.primaryReleased && inside)
		{
			setFocused(true);
			return EventResult::Consume;
		}

		return inside || hasKeyboardFocus() ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult TextField::handleChar(uint32_t codepoint)
	{
		if (!hasKeyboardFocus())
		{
			return EventResult::Ignore;
		}

		if (codepoint < 32 || codepoint >= 127)
		{
			return EventResult::Consume;
		}

		_text.insert(_text.begin() + static_cast<std::ptrdiff_t>(_caret), static_cast<char>(codepoint));
		++_caret;
		notifyChanged();
		return EventResult::Consume;
	}

	EventResult TextField::handleKey(TextKey key)
	{
		if (!hasKeyboardFocus())
		{
			return EventResult::Ignore;
		}

		switch (key)
		{
		case TextKey::Backspace:
			if (_caret > 0)
			{
				_text.erase(_caret - 1, 1);
				--_caret;
				notifyChanged();
			}
			break;
		case TextKey::Delete:
			if (_caret < _text.size())
			{
				_text.erase(_caret, 1);
				notifyChanged();
			}
			break;
		case TextKey::Left:
			if (_caret > 0)
			{
				--_caret;
			}
			break;
		case TextKey::Right:
			if (_caret < _text.size())
			{
				++_caret;
			}
			break;
		case TextKey::Home:
			_caret = 0;
			break;
		case TextKey::End:
			_caret = _text.size();
			break;
		case TextKey::Enter:
			break;
		}

		return EventResult::Consume;
	}

	void TextField::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ 280.0f, 36.0f };
		}

		_typeface->layoutText(_text, fontSize, _glyphs);
	}

	void TextField::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		paint.fill(box, background);
		if (hasKeyboardFocus())
		{
			const float inset = 1.0f;
			if (box.size.x > inset * 2.0f && box.size.y > inset * 2.0f)
			{
				paint.fill(
					Box{
						box.pos.x + inset,
						box.pos.y + inset,
						box.size.x - inset * 2.0f,
						box.size.y - inset * 2.0f
					},
					glm::vec3{ background.x + 0.08f, background.y + 0.08f, background.z + 0.08f });
			}
		}

		const float innerWidth = std::max(1.0f, box.size.x - _padding * 2.0f);
		const float caret = caretX();
		const float scrollX = caret > innerWidth ? caret - innerWidth : 0.0f;

		paint.pushClip(box);
		const glm::vec2 origin{
			box.pos.x + _padding - scrollX,
			box.pos.y + _padding
		};
		paint.glyphs(_glyphs, origin, color);

		if (hasKeyboardFocus())
		{
			const float x = origin.x + caret;
			const float h = std::max(12.0f, box.size.y - _padding * 2.0f);
			paint.fill(Box{ x, box.pos.y + _padding, 2.0f, h }, caretColor);
		}

		paint.popClip();
	}

	void TextField::notifyChanged()
	{
		if (onChanged)
		{
			onChanged(_text);
		}
	}

	float TextField::caretX() const
	{
		std::vector<GlyphVertex> prefix;
		const glm::vec2 size = _typeface->layoutText(_text.substr(0, _caret), fontSize, prefix);
		return size.x;
	}
}
