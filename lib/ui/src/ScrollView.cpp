#include <helsinki/Ui/ScrollView.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		constexpr float kBarWidth = 10.0f;
		constexpr float kMinThumb = 16.0f;

		class VerticalBar : public Widget
		{
		public:
			explicit VerticalBar(Node& node, ScrollView& owner) :
				Widget(node),
				_owner(&owner)
			{
				hitTestEnabled = true;
				focusable = false;
			}

			Box thumbBox() const
			{
				const auto& box = node().world;
				Node& view = _owner->viewport();
				const float contentH = std::max(view.contentSize().y, view.world.size.y);
				const float viewH = std::max(view.world.size.y, 1.0f);
				const float trackH = box.size.y;
				const float thumbH = std::clamp(trackH * (viewH / contentH), kMinThumb, trackH);
				const float maxS = view.maxScroll().y;
				const float t = maxS > 0.0f ? view.scrollOffset.y / maxS : 0.0f;
				const float y = box.pos.y + t * (trackH - thumbH);
				return Box{ box.pos.x, y, box.size.x, thumbH };
			}

			void prepare() override
			{
				if (!visible)
				{
					node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
					return;
				}

				node().intrinsicSize = glm::vec2{ kBarWidth, _owner->viewport().world.size.y };
			}

			EventResult handle(const Pointer& pointer) override
			{
				if (!visible)
				{
					return EventResult::Ignore;
				}

				const bool inside = node().world.contains(pointer.position);
				const Box thumb = thumbBox();

				if (pointer.primaryDown && (inside || hasPointerCapture()))
				{
					capturePointer();
					applyPointer(pointer, thumb);
					return EventResult::Consume;
				}

				if (hasPointerCapture() && !pointer.primaryDown)
				{
					releasePointer();
					return EventResult::Consume;
				}

				return inside ? EventResult::Consume : EventResult::Ignore;
			}

			void paint(IPaint& paint) const override
			{
				if (!visible)
				{
					return;
				}

				paint.fill(node().world, _owner->trackColor);
				paint.fill(thumbBox(), _owner->thumbColor);
			}

		private:
			void applyPointer(const Pointer& pointer, const Box& thumb)
			{
				Node& view = _owner->viewport();
				const auto& track = node().world;
				const float thumbH = thumb.size.y;
				const float travel = std::max(track.size.y - thumbH, 1.0f);
				const float y = pointer.position.y - track.pos.y - thumbH * 0.5f;
				const float t = std::clamp(y / travel, 0.0f, 1.0f);
				view.scrollOffset.y = t * view.maxScroll().y;
				view.clampScroll();
			}

			ScrollView* _owner = nullptr;
		};
	}

	ScrollView::ScrollView(Node& node) :
		Widget(node)
	{
		hitTestEnabled = false;
		focusable = false;
		node.kind = Kind::Absolute;

		_viewport = &node.addChild();
		_viewport->kind = Kind::Column;
		_viewport->clip = true;
		_viewport->gap = 8.0f;

		_vBar = std::make_unique<VerticalBar>(node.addChild(), *this);
		_vBar->visible = false;
	}

	bool ScrollView::barVisible() const
	{
		return _vBar->visible;
	}

	void ScrollView::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = viewportSize;
		}
	}

	void ScrollView::afterLayout()
	{
		layoutChrome();
	}

	glm::vec2 ScrollView::resolvedViewportSize() const
	{
		glm::vec2 size = viewportSize;
		if (node().world.size.x > size.x)
		{
			size.x = node().world.size.x;
		}

		if (node().world.size.y > size.y)
		{
			size.y = node().world.size.y;
		}

		return size;
	}

	void ScrollView::layoutChrome()
	{
		const glm::vec2 size = resolvedViewportSize();
		const float contentH = _viewport->measure().y;
		const bool overflow = scrollBars && contentH > size.y + 0.5f;
		_vBar->visible = overflow;

		const float barW = overflow ? kBarWidth : 0.0f;
		_viewport->setTopLeft({ std::max(1.0f, size.x - barW), size.y });
		_viewport->relative = { 0.0f, 0.0f };
		_vBar->node().setTopLeft({ barW, size.y });
		_vBar->node().relative = { size.x - barW, 0.0f };

		node().intrinsicSize = size;
		_viewport->arrange(node().world);
		_vBar->node().arrange(node().world);
		_viewport->clampScroll();
	}
}
