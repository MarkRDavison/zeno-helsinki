#include <helsinki/Ui/Tabs.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		constexpr float kChevronWidth = 24.0f;
		constexpr float kHeaderPadX = 10.0f;

		class TabHeader : public Widget
		{
		public:
			TabHeader(Node& node, Tabs& owner, int index, const ITypeface& typeface) :
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
				layoutTitle();
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
				const bool selected = _index == _owner->selectedIndex();
				const bool hovered = _index == _owner->hoverIndex();
				const glm::vec3 bg = selected
					? _owner->selectedColor
					: hovered
						? _owner->hoverColor
						: _owner->fillColor;
				paint.fill(box, bg);
				paint.glyphs(
					_glyphs,
					box.pos + glm::vec2{ kHeaderPadX, (box.size.y - 16.0f) * 0.5f },
					_owner->color);
			}

			void setTitle(const std::string& title)
			{
				_title = title;
			}

			void layoutTitle()
			{
				const glm::vec2 textSize = _typeface->layoutText(
					_title,
					_owner->resolvedFontSize(),
					_glyphs);
				node().intrinsicSize = glm::vec2{
					textSize.x + kHeaderPadX * 2.0f,
					std::max(_owner->headerHeight, textSize.y + 8.0f)
				};
			}

		private:
			Tabs* _owner = nullptr;
			int _index = 0;
			const ITypeface* _typeface = nullptr;
			std::string _title;
			std::vector<GlyphVertex> _glyphs;
		};

		class PageHost : public Widget
		{
		public:
			PageHost(Node& node, Tabs& owner, int index) :
				Widget(node),
				_owner(&owner),
				_index(index)
			{
				hitTestEnabled = false;
				focusable = false;
			}

			void paint(IPaint& paint) const override
			{
				if (!visible)
				{
					return;
				}

				paint.fill(node().world, glm::vec3{ 0.14f, 0.15f, 0.19f });
			}

			void prepare() override
			{
				const bool on = _index == _owner->selectedIndex();
				visible = on;
				hitTestEnabled = false;
				Node* body = node().parent();
				const bool fill = body != nullptr && body->world.size.y > 1.0f;
				if (!on)
				{
					node().kind = Kind::Absolute;
					node().clip = false;
					node().anchorMin = { 0.0f, 0.0f };
					node().anchorMax = { 0.0f, 0.0f };
					node().offset = {};
					node().relative = { 0.0f, 0.0f };
					node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
				}
				else if (fill)
				{
					node().setFillParent();
					node().kind = Kind::Column;
					node().clip = true;
					node().intrinsicSize.reset();
				}
				else
				{
					node().kind = Kind::Column;
					node().clip = false;
					node().anchorMin = { 0.0f, 0.0f };
					node().anchorMax = { 0.0f, 0.0f };
					node().offset = {};
					node().intrinsicSize.reset();
				}
			}

		private:
			Tabs* _owner = nullptr;
			int _index = 0;
		};

		class Chevron : public Widget
		{
		public:
			Chevron(Node& node, Tabs& owner, int direction) :
				Widget(node),
				_owner(&owner),
				_direction(direction)
			{
				hitTestEnabled = true;
				focusable = false;
			}

			void prepare() override
			{
				if (!visible)
				{
					node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
					return;
				}

				node().intrinsicSize = glm::vec2{ kChevronWidth, _owner->headerHeight };
			}

			EventResult handle(const Pointer& pointer) override
			{
				if (!visible || !node().world.contains(pointer.position))
				{
					return EventResult::Ignore;
				}

				if (pointer.primaryReleased)
				{
					_owner->scrollHeaders(_direction);
				}

				return EventResult::Consume;
			}

			void paint(IPaint& paint) const override
			{
				if (!visible)
				{
					return;
				}

				const auto& box = node().world;
				paint.fill(box, _owner->fillColor);

				const bool atEnd = _direction < 0
					? _owner->headerClip().scrollOffset.x <= 0.0f
					: _owner->headerClip().scrollOffset.x >= _owner->headerClip().maxScroll().x;
				const glm::vec3 col = atEnd
					? glm::vec3{ 0.45f, 0.46f, 0.50f }
					: _owner->color;

				const float cx = box.pos.x + box.size.x * 0.5f;
				const float cy = box.pos.y + box.size.y * 0.5f;
				for (int i = 0; i < 4; ++i)
				{
					const float t = static_cast<float>(i);
					const float h = _direction < 0
						? 2.0f + t * 2.0f
						: 8.0f - t * 2.0f;
					const float x = cx - 4.0f + t * 2.0f;
					paint.fill(Box{ x, cy - h * 0.5f, 2.0f, h }, col);
				}
			}

		private:
			Tabs* _owner = nullptr;
			int _direction = 1;
		};
	}

	Tabs::Tabs(Node& node, const ITypeface& typeface) :
		Widget(node),
		_typeface(&typeface)
	{
		hitTestEnabled = false;
		focusable = true;
		node.kind = Kind::Absolute;

		_bar = &node.addChild();
		_bar->kind = Kind::Absolute;
		_bar->anchorMin = { 0.0f, 0.0f };
		_bar->anchorMax = { 1.0f, 0.0f };
		_bar->offset.bottom = headerHeight;

		_prev = std::make_unique<Chevron>(_bar->addChild(), *this, -1);
		_clip = &_bar->addChild();
		_clip->kind = Kind::Row;
		_clip->clip = true;
		_clip->gap = 4.0f;
		_next = std::make_unique<Chevron>(_bar->addChild(), *this, 1);

		_body = &node.addChild();
		_body->kind = Kind::Absolute;
		_body->anchorMin = { 0.0f, 0.0f };
		_body->anchorMax = { 1.0f, 1.0f };
		_body->offset.top = headerHeight;
	}

	Node& Tabs::addPage(std::string title)
	{
		const int index = pageCount();
		_titles.push_back(std::move(title));

		auto header = std::make_unique<TabHeader>(_clip->addChild(), *this, index, *_typeface);
		static_cast<TabHeader*>(header.get())->setTitle(_titles.back());
		_headers.push_back(std::move(header));

		Node& pageNode = _body->addChild();
		pageNode.kind = Kind::Column;
		pageNode.gap = 8.0f;
		_pageHosts.push_back(std::make_unique<PageHost>(pageNode, *this, index));
		_pages.push_back(&pageNode);
		applyPageVisibility();
		return pageNode;
	}

	Node& Tabs::page(int index)
	{
		return *_pages.at(static_cast<std::size_t>(index));
	}

	void Tabs::setSelectedIndex(int index)
	{
		if (_pages.empty())
		{
			_selected = 0;
			return;
		}

		_selected = std::clamp(index, 0, pageCount() - 1);
		applyPageVisibility();
		ensureSelectedVisible();
	}

	void Tabs::pick(int index)
	{
		const int previous = _selected;
		setSelectedIndex(index);
		setFocused(true);
		if (previous != _selected && onChanged)
		{
			onChanged(_selected);
		}
	}

	void Tabs::prepare()
	{
		_hover = -1;
		for (auto& header : _headers)
		{
			static_cast<TabHeader*>(header.get())->layoutTitle();
		}

		syncBodyAnchors();

		const bool overflow = headerContentWidth() > barWidth();
		_overflow = overflow;
		_prev->visible = overflow;
		_next->visible = overflow;
		if (!overflow)
		{
			_clip->scrollOffset = { 0.0f, 0.0f };
		}
	}

	void Tabs::afterLayout()
	{
		syncBodyAnchors();
		if (node().world.size.y > headerHeight)
		{
			_body->arrange(node().world);
		}

		layoutHeaderStrip();
	}

	EventResult Tabs::handleKey(TextKey key)
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

	void Tabs::setHoverIndex(int index)
	{
		_hover = index;
	}

	void Tabs::scrollHeaders(int direction)
	{
		const float step = std::max(_clip->world.size.x * 0.5f, 40.0f);
		_clip->scrollOffset.x += static_cast<float>(direction) * step;
		_clip->clampScroll();
		_clip->arrange(_bar->world);
	}

	void Tabs::applyPageVisibility()
	{
		for (int i = 0; i < pageCount(); ++i)
		{
			Widget* host = _pageHosts[static_cast<std::size_t>(i)].get();
			const bool on = i == _selected;
			host->visible = on;
			Node& page = *_pages[static_cast<std::size_t>(i)];
			const bool fill = _body->world.size.y > 1.0f;
			if (!on)
			{
				page.kind = Kind::Absolute;
				page.clip = false;
				page.anchorMin = { 0.0f, 0.0f };
				page.anchorMax = { 0.0f, 0.0f };
				page.offset = {};
				page.relative = { 0.0f, 0.0f };
				page.intrinsicSize = glm::vec2{ 0.0f, 0.0f };
			}
			else if (fill)
			{
				page.setFillParent();
				page.kind = Kind::Column;
				page.clip = true;
				page.intrinsicSize.reset();
			}
			else
			{
				page.kind = Kind::Column;
				page.clip = false;
				page.anchorMin = { 0.0f, 0.0f };
				page.anchorMax = { 0.0f, 0.0f };
				page.offset = {};
				page.intrinsicSize.reset();
			}
		}
	}

	void Tabs::ensureSelectedVisible()
	{
		if (_headers.empty() || !_overflow)
		{
			return;
		}

		const auto& tab = _headers[static_cast<std::size_t>(_selected)]->node().world;
		const auto& clipBox = _clip->world;
		if (clipBox.size.x <= 0.0f)
		{
			return;
		}

		const float tabRight = tab.pos.x + tab.size.x;
		const float clipRight = clipBox.pos.x + clipBox.size.x;
		if (tabRight <= clipBox.pos.x)
		{
			_clip->scrollOffset.x -= clipBox.pos.x - tab.pos.x;
		}
		else if (tab.pos.x >= clipRight)
		{
			_clip->scrollOffset.x += tabRight - clipRight;
		}

		_clip->clampScroll();
	}

	void Tabs::moveSelection(int delta)
	{
		if (_pages.empty())
		{
			return;
		}

		const int n = pageCount();
		pick((_selected + delta % n + n) % n);
	}

	float Tabs::headerContentWidth() const
	{
		float width = 0.0f;
		const auto& kids = _clip->children();
		for (std::size_t i = 0; i < kids.size(); ++i)
		{
			const glm::vec2 size = kids[i]->intrinsicSize.value_or(kids[i]->world.size);
			width += size.x;
			if (i + 1 < kids.size())
			{
				width += _clip->gap;
			}
		}

		return width;
	}

	void Tabs::syncBodyAnchors()
	{
		_bar->offset.bottom = headerHeight;
		if (node().world.size.y > headerHeight)
		{
			_body->anchorMax = { 1.0f, 1.0f };
			_body->offset.top = headerHeight;
			_body->offset.bottom = 0.0f;
		}
		else
		{
			_body->anchorMax = { 1.0f, 0.0f };
			_body->offset.top = headerHeight;
			_body->offset.bottom = headerHeight;
		}
	}

	float Tabs::barWidth() const
	{
		if (_bar->world.size.x > 1.0f)
		{
			return _bar->world.size.x;
		}

		if (node().world.size.x > 1.0f)
		{
			return node().world.size.x;
		}

		return maxHeaderWidth;
	}

	void Tabs::layoutHeaderStrip()
	{
		const float contentW = headerContentWidth();
		const float width = barWidth();
		_overflow = contentW > width;
		_prev->visible = _overflow;
		_next->visible = _overflow;
		if (!_overflow)
		{
			_clip->scrollOffset = { 0.0f, 0.0f };
		}

		const float btn = _overflow ? kChevronWidth : 0.0f;
		const float clipW = std::max(16.0f, width - btn * 2.0f);
		const float h = headerHeight;

		_prev->node().setTopLeft({ btn, h });
		_prev->node().relative = { 0.0f, 0.0f };
		_clip->setTopLeft({ clipW, h });
		_clip->relative = { btn, 0.0f };
		_next->node().setTopLeft({ btn, h });
		_next->node().relative = { btn + clipW, 0.0f };

		_prev->node().arrange(_bar->world);
		_clip->arrange(_bar->world);
		_next->node().arrange(_bar->world);
		_clip->clampScroll();
	}
}
