#include <helsinki/Ui/Dropdown.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		Dropdown* gOpenDropdown = nullptr;

		class ItemRow : public Widget
		{
		public:
			ItemRow(Node& node, Dropdown& owner, int index, const ITypeface& typeface) :
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
				_typeface->layoutText(text, _owner->resolvedFontSize(), _glyphs);
				node().intrinsicSize = glm::vec2{ 200.0f, _owner->resolvedItemHeight() };
			}

			EventResult handle(const Pointer& pointer) override
			{
				const bool inside = node().world.contains(pointer.position);
				if (inside)
				{
					_owner->setHighlight(_index);
					if (pointer.primaryReleased)
					{
						_owner->pick(_index);
					}

					return EventResult::Consume;
				}

				return EventResult::Ignore;
			}

			void paint(IPaint& paint) const override
			{
				const auto& box = node().world;
				const bool hot = _index == _owner->highlightIndex();
				const bool selected = _index == _owner->selectedIndex();
				const glm::vec3 bg = hot
					? _owner->resolvedHighlightColor()
					: (selected ? _owner->resolvedHoverColor() : _owner->resolvedFillColor());
				paint.fill(box, bg);
				const glm::vec2 origin{ box.pos.x + 8.0f, box.pos.y + 4.0f };
				paint.glyphs(_glyphs, origin, _owner->resolvedColor());
			}

		private:
			Dropdown* _owner = nullptr;
			int _index = 0;
			const ITypeface* _typeface = nullptr;
			std::vector<GlyphVertex> _glyphs;
			std::string _empty;
		};

		bool overlayContains(const Dropdown& dropdown, Widget* hit)
		{
			if (hit == nullptr)
			{
				return false;
			}

			if (hit == &dropdown)
			{
				return true;
			}

			for (const Node* n = &hit->node(); n != nullptr; n = n->parent())
			{
				if (n == &dropdown.node() || n == &dropdown.overlay())
				{
					return true;
				}
			}

			return false;
		}
	}

	void dismissOpenOverlay(Widget* hit)
	{
		if (gOpenDropdown == nullptr)
		{
			return;
		}

		if (!overlayContains(*gOpenDropdown, hit))
		{
			gOpenDropdown->setOpen(false);
		}
	}

	Dropdown::Dropdown(Node& header, const ITypeface& typeface) :
		Widget(header),
		_typeface(&typeface)
	{
		hitTestEnabled = true;
		focusable = true;
		Node* root = treeRoot();
		_overlay = &root->addChild();
		_overlay->kind = Kind::Column;
		_overlay->clip = true;
		_overlay->gap = 0.0f;
		_overlay->padding = Edges::all(theme().padding);
		_overlayPanel = std::make_unique<Panel>(*_overlay);
		_overlayPanel->color = resolvedFillColor();
		_overlayPanel->hitTestEnabled = false;
		syncOverlayHitTest();
	}

	Dropdown::~Dropdown()
	{
		if (gOpenDropdown == this)
		{
			gOpenDropdown = nullptr;
		}
	}

	Node* Dropdown::treeRoot()
	{
		Node* root = &node();
		while (root->parent() != nullptr)
		{
			root = root->parent();
		}

		return root;
	}

	void Dropdown::setItems(std::vector<std::string> items)
	{
		_items = std::move(items);
		if (_selected >= static_cast<int>(_items.size()))
		{
			_selected = _items.empty() ? 0 : static_cast<int>(_items.size()) - 1;
		}

		_highlight = _selected;
		rebuildRows();
	}

	void Dropdown::setSelectedIndex(int index)
	{
		if (_items.empty())
		{
			_selected = 0;
			return;
		}

		_selected = std::clamp(index, 0, static_cast<int>(_items.size()) - 1);
		_highlight = _selected;
	}

	void Dropdown::setOpen(bool open)
	{
		if (open)
		{
			if (_items.empty())
			{
				return;
			}

			if (gOpenDropdown != nullptr && gOpenDropdown != this)
			{
				gOpenDropdown->setOpen(false);
			}

			_open = true;
			_highlight = _selected;
			gOpenDropdown = this;
			attachOverlayLast();
		}
		else
		{
			_open = false;
			if (gOpenDropdown == this)
			{
				gOpenDropdown = nullptr;
			}
		}

		syncOverlayHitTest();
	}

	void Dropdown::setFocused(bool focused)
	{
		if (!focused)
		{
			setOpen(false);
		}

		Widget::setFocused(focused);
	}

	void Dropdown::prepare()
	{
		_hovered = false;
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = glm::vec2{ theme().controlWidth, theme().controlHeight };
		}

		_typeface->layoutText(selectedText(), resolvedFontSize(), _glyphs);
		_overlay->padding = Edges::all(theme().padding);
		if (_overlayPanel)
		{
			_overlayPanel->color = resolvedFillColor();
		}
	}

	void Dropdown::afterLayout()
	{
		Node* root = treeRoot();
		const auto& header = node().world;
		const float listHeight = _open
			? std::min(contentHeight(), resolvedMaxListHeight())
			: 0.0f;
		_overlay->setTopLeft({ header.size.x, listHeight });
		_overlay->relative = header.pos - root->world.pos + glm::vec2{ 0.0f, header.size.y };
		_overlay->scrollOffset = _open ? _overlay->scrollOffset : glm::vec2{ 0.0f, 0.0f };
		_overlay->arrange(root->world);
		_overlay->clampScroll();
	}

	EventResult Dropdown::handle(const Pointer& pointer)
	{
		_hovered = node().world.contains(pointer.position);
		if (_hovered && pointer.primaryReleased)
		{
			setFocused(true);
			setOpen(!_open);
			return EventResult::Consume;
		}

		return _hovered || _open ? EventResult::Consume : EventResult::Ignore;
	}

	EventResult Dropdown::handleKey(TextKey key)
	{
		if (!hasKeyboardFocus())
		{
			return EventResult::Ignore;
		}

		switch (key)
		{
		case TextKey::Escape:
			setOpen(false);
			return EventResult::Consume;
		case TextKey::Down:
			if (!_open)
			{
				setOpen(true);
			}
			else if (!_items.empty())
			{
				_highlight = std::min(_highlight + 1, static_cast<int>(_items.size()) - 1);
			}
			return EventResult::Consume;
		case TextKey::Up:
			if (_open && !_items.empty())
			{
				_highlight = std::max(_highlight - 1, 0);
			}
			return EventResult::Consume;
		case TextKey::Enter:
		case TextKey::Space:
			if (_open)
			{
				pick(_highlight);
			}
			else
			{
				setOpen(true);
			}
			return EventResult::Consume;
		default:
			return EventResult::Ignore;
		}
	}

	void Dropdown::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		const glm::vec3 bg = _hovered || (hasKeyboardFocus() && !_open)
			? resolvedHoverColor()
			: resolvedFillColor();
		paint.fill(box, bg);
		const glm::vec2 origin{ box.pos.x + 8.0f, box.pos.y + 6.0f };
		paint.glyphs(_glyphs, origin, resolvedColor());

		const float caret = 8.0f;
		const float cx = box.pos.x + box.size.x - 14.0f;
		const float cy = box.pos.y + box.size.y * 0.5f;
		for (int i = 0; i < 4; ++i)
		{
			const float w = _open ? 2.0f + static_cast<float>(i) * 2.0f : caret - static_cast<float>(i) * 2.0f;
			const float x = cx - w * 0.5f;
			const float y = _open
				? cy + 3.0f - static_cast<float>(i) * 2.0f
				: cy - 4.0f + static_cast<float>(i) * 2.0f;
			paint.fill(Box{ x, y, w, 2.0f }, resolvedColor());
		}
	}

	void Dropdown::setHighlight(int index)
	{
		if (_items.empty())
		{
			_highlight = 0;
			return;
		}

		_highlight = std::clamp(index, 0, static_cast<int>(_items.size()) - 1);
	}

	void Dropdown::pick(int index)
	{
		setSelectedIndex(index);
		if (onChanged)
		{
			onChanged(_selected);
		}

		setOpen(false);
	}

	void Dropdown::rebuildRows()
	{
		_rows.clear();
		while (!_overlay->children().empty())
		{
			_overlay->releaseChild(*_overlay->children().front());
		}

		for (int i = 0; i < static_cast<int>(_items.size()); ++i)
		{
			_rows.push_back(std::make_unique<ItemRow>(_overlay->addChild(), *this, i, *_typeface));
		}

		syncOverlayHitTest();
	}

	void Dropdown::attachOverlayLast()
	{
		Node* root = treeRoot();
		if (_overlay->parent() == root && !root->children().empty()
			&& root->children().back().get() == _overlay)
		{
			return;
		}

		if (_overlay->parent() == nullptr)
		{
			return;
		}

		auto held = _overlay->parent()->releaseChild(*_overlay);
		if (held)
		{
			root->addChild(std::move(held));
		}
	}

	void Dropdown::syncOverlayHitTest()
	{
		if (_overlayPanel)
		{
			_overlayPanel->hitTestEnabled = false;
		}

		for (auto& row : _rows)
		{
			row->hitTestEnabled = _open;
		}
	}

	float Dropdown::contentHeight() const
	{
		const float n = static_cast<float>(_items.size());
		const float gaps = n > 0.0f ? _overlay->gap * (n - 1.0f) : 0.0f;
		return n * resolvedItemHeight() + gaps + _overlay->padding.top + _overlay->padding.bottom;
	}

	const std::string& Dropdown::selectedText() const
	{
		static const std::string empty;
		if (_items.empty() || _selected < 0 || _selected >= static_cast<int>(_items.size()))
		{
			return empty;
		}

		return _items[static_cast<std::size_t>(_selected)];
	}
}
