#include <helsinki/Ui/RadioGroup.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		class RadioOption : public Widget
		{
		public:
			RadioOption(Node& node, RadioGroup& owner, int index, const ITypeface& typeface) :
				Widget(node),
				_owner(&owner),
				_index(index),
				_typeface(&typeface)
			{
				hitTestEnabled = true;
				focusable = false;
			}

			void prepare() override
			{
				const auto& items = _owner->items();
				const std::string& text = items.empty() ? _empty : items[static_cast<std::size_t>(_index)];
				const glm::vec2 textSize = _typeface->layoutText(text, _owner->fontSize, _glyphs);
				const float height = std::max(_owner->markSize, textSize.y + 4.0f);
				node().intrinsicSize = glm::vec2{
					_owner->markSize + _owner->labelGap + textSize.x,
					height
				};
			}

			EventResult handle(const Pointer& pointer) override
			{
				const bool inside = node().world.contains(pointer.position);
				if (!inside)
				{
					return EventResult::Ignore;
				}

				_owner->setHoverIndex(_index);
				if (pointer.primaryReleased)
				{
					_owner->pick(_index);
				}

				return EventResult::Consume;
			}

			void paint(IPaint& paint) const override
			{
				const auto& box = node().world;
				const bool hovered = _index == _owner->hoverIndex();
				const bool selected = _index == _owner->selectedIndex();
				if (hovered)
				{
					paint.fill(box, _owner->hoverColor);
				}

				const float mark = _owner->markSize;
				const float markY = box.pos.y + (box.size.y - mark) * 0.5f;
				const glm::vec3 well = selected || (_owner->hasKeyboardFocus() && selected)
					? glm::vec3{
						std::min(1.0f, _owner->wellColor.x + 0.18f),
						std::min(1.0f, _owner->wellColor.y + 0.18f),
						std::min(1.0f, _owner->wellColor.z + 0.18f)
					}
					: _owner->wellColor;
				paint.fill(Box{ box.pos.x, markY, mark, mark }, well);

				const float inner = mark * 0.22f;
				paint.fill(
					Box{
						box.pos.x + inner,
						markY + inner,
						mark - inner * 2.0f,
						mark - inner * 2.0f
					},
					hovered ? _owner->hoverColor : glm::vec3{ 0.16f, 0.17f, 0.22f });

				if (selected)
				{
					const float dot = mark * 0.34f;
					const float dotInset = (mark - dot) * 0.5f;
					paint.fill(
						Box{
							box.pos.x + dotInset,
							markY + dotInset,
							dot,
							dot
						},
						hovered ? _owner->highlightColor : _owner->selectedColor);
				}

				const glm::vec2 origin{
					box.pos.x + mark + _owner->labelGap,
					box.pos.y + (box.size.y - 16.0f) * 0.5f
				};
				paint.glyphs(_glyphs, origin, _owner->color);
			}

		private:
			RadioGroup* _owner = nullptr;
			int _index = 0;
			const ITypeface* _typeface = nullptr;
			std::vector<GlyphVertex> _glyphs;
			std::string _empty;
		};
	}

	RadioGroup::RadioGroup(Node& node, const ITypeface& typeface) :
		Widget(node),
		_typeface(&typeface)
	{
		hitTestEnabled = false;
		focusable = true;
		node.gap = 8.0f;
		node.crossAlign = Align::Start;
		applyOrientation();
	}

	void RadioGroup::setItems(std::vector<std::string> items)
	{
		_items = std::move(items);
		if (_selected >= static_cast<int>(_items.size()))
		{
			_selected = _items.empty() ? 0 : static_cast<int>(_items.size()) - 1;
		}

		rebuildOptions();
	}

	void RadioGroup::setSelectedIndex(int index)
	{
		if (_items.empty())
		{
			_selected = 0;
			return;
		}

		_selected = std::clamp(index, 0, static_cast<int>(_items.size()) - 1);
	}

	void RadioGroup::setOrientation(RadioOrientation orientation)
	{
		_orientation = orientation;
		applyOrientation();
	}

	void RadioGroup::prepare()
	{
		_hover = -1;
	}

	EventResult RadioGroup::handleKey(TextKey key)
	{
		if (!hasKeyboardFocus())
		{
			return EventResult::Ignore;
		}

		switch (key)
		{
		case TextKey::Up:
		case TextKey::Left:
			moveSelection(-1);
			return EventResult::Consume;
		case TextKey::Down:
		case TextKey::Right:
			moveSelection(1);
			return EventResult::Consume;
		case TextKey::Enter:
		case TextKey::Space:
			return EventResult::Consume;
		default:
			return EventResult::Ignore;
		}
	}

	void RadioGroup::pick(int index)
	{
		const int previous = _selected;
		setSelectedIndex(index);
		setFocused(true);
		if (previous != _selected && onChanged)
		{
			onChanged(_selected);
		}
	}

	void RadioGroup::setHoverIndex(int index)
	{
		_hover = index;
	}

	void RadioGroup::rebuildOptions()
	{
		_options.clear();
		while (!node().children().empty())
		{
			node().releaseChild(*node().children().front());
		}

		for (int i = 0; i < static_cast<int>(_items.size()); ++i)
		{
			_options.push_back(std::make_unique<RadioOption>(node().addChild(), *this, i, *_typeface));
		}
	}

	void RadioGroup::moveSelection(int delta)
	{
		if (_items.empty())
		{
			return;
		}

		const int n = static_cast<int>(_items.size());
		pick((_selected + delta % n + n) % n);
	}

	void RadioGroup::applyOrientation()
	{
		node().kind = _orientation == RadioOrientation::Horizontal
			? Kind::Row
			: Kind::Column;
	}
}
