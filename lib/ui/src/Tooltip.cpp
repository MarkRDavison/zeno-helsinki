#include <helsinki/Ui/Tooltip.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		Tooltip* gTooltip = nullptr;
		constexpr float kPadding = 8.0f;

		Widget* tooltipSource(Widget* hit)
		{
			for (Widget* widget = hit; widget != nullptr; widget = widget->parentWidget())
			{
				if (!widget->tooltip.empty())
				{
					return widget;
				}
			}

			return nullptr;
		}
	}

	void syncTooltip(Widget* hit)
	{
		if (gTooltip != nullptr)
		{
			gTooltip->syncFromHit(hit);
		}
	}

	Tooltip::Tooltip(Node& overlayNode, const ITypeface& typeface) :
		Widget(overlayNode),
		_typeface(&typeface)
	{
		hitTestEnabled = false;
		focusable = false;
		node().setTopLeft({ 0.0f, 0.0f });
		gTooltip = this;
	}

	Tooltip::~Tooltip()
	{
		if (gTooltip == this)
		{
			gTooltip = nullptr;
		}
	}

	float Tooltip::resolvedDelay() const
	{
		return delay.value_or(theme().tooltipDelay);
	}

	float Tooltip::resolvedOffset() const
	{
		return offset.value_or(theme().tooltipOffset);
	}

	TooltipPlacement Tooltip::resolvedPlacement() const
	{
		return placement.value_or(theme().tooltipPlacement);
	}

	void Tooltip::tick(float dt)
	{
		if (_pending != _hoverSource)
		{
			_hoverSource = _pending;
			_accum = 0.0f;
			hide();
		}

		if (_pending == nullptr)
		{
			_accum = 0.0f;
			hide();
			return;
		}

		_accum += dt;
		if (_accum >= resolvedDelay())
		{
			show(_pending);
		}
	}

	void Tooltip::syncFromHit(Widget* hit)
	{
		_pending = tooltipSource(hit);
		if (_pending != _hoverSource)
		{
			_hoverSource = _pending;
			_accum = 0.0f;
			hide();
		}

		if (_pending == nullptr)
		{
			hide();
			return;
		}

		if (resolvedDelay() <= 0.0f)
		{
			show(_pending);
		}
	}

	void Tooltip::prepare()
	{
		if (!_visible)
		{
			_glyphs.clear();
			node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
			return;
		}

		measureText();
	}

	void Tooltip::afterLayout()
	{
		Node* root = treeRoot();
		if (!_visible || _source == nullptr)
		{
			node().setTopLeft({ 0.0f, 0.0f });
			node().relative = { 0.0f, 0.0f };
			node().arrange(root->world);
			return;
		}

		const glm::vec2 size = node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		const auto& target = _source->node().world;
		const auto& rootBox = root->world;
		const float gap = resolvedOffset();

		float x = target.pos.x;
		float y = target.pos.y;
		switch (resolvedPlacement())
		{
		case TooltipPlacement::Below:
			y = target.pos.y + target.size.y + gap;
			if (y + size.y > rootBox.pos.y + rootBox.size.y)
			{
				y = target.pos.y - gap - size.y;
			}
			break;
		case TooltipPlacement::Above:
			y = target.pos.y - gap - size.y;
			if (y < rootBox.pos.y)
			{
				y = target.pos.y + target.size.y + gap;
			}
			break;
		case TooltipPlacement::Right:
			x = target.pos.x + target.size.x + gap;
			if (x + size.x > rootBox.pos.x + rootBox.size.x)
			{
				x = target.pos.x - gap - size.x;
			}
			break;
		case TooltipPlacement::Left:
			x = target.pos.x - gap - size.x;
			if (x < rootBox.pos.x)
			{
				x = target.pos.x + target.size.x + gap;
			}
			break;
		}

		const float minX = rootBox.pos.x;
		const float maxX = rootBox.pos.x + std::max(0.0f, rootBox.size.x - size.x);
		const float minY = rootBox.pos.y;
		const float maxY = rootBox.pos.y + std::max(0.0f, rootBox.size.y - size.y);
		x = std::clamp(x, minX, maxX);
		y = std::clamp(y, minY, maxY);

		node().setTopLeft(size);
		node().relative = glm::vec2{ x, y } - rootBox.pos;
		node().arrange(rootBox);
	}

	void Tooltip::paint(IPaint& paint) const
	{
		if (!_visible)
		{
			return;
		}

		paint.fill(node().world, fillColor);
		paint.glyphs(_glyphs, node().world.pos + glm::vec2{ kPadding, kPadding }, color);
	}

	Node* Tooltip::treeRoot()
	{
		Node* root = &node();
		while (root->parent() != nullptr)
		{
			root = root->parent();
		}

		return root;
	}

	void Tooltip::attachLast()
	{
		Node* root = treeRoot();
		if (node().parent() == root && !root->children().empty()
			&& root->children().back().get() == &node())
		{
			return;
		}

		if (node().parent() == nullptr)
		{
			return;
		}

		auto held = node().parent()->releaseChild(node());
		if (held)
		{
			root->addChild(std::move(held));
		}
	}

	void Tooltip::show(Widget* source)
	{
		if (source == nullptr)
		{
			return;
		}

		_source = source;
		_text = source->tooltip;
		_visible = true;
		measureText();
		attachLast();
		afterLayout();
	}

	void Tooltip::hide()
	{
		if (!_visible && _source == nullptr)
		{
			return;
		}

		_visible = false;
		_source = nullptr;
		_text.clear();
		_glyphs.clear();
		node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
		node().setTopLeft({ 0.0f, 0.0f });
		if (node().parent() != nullptr)
		{
			afterLayout();
		}
	}

	void Tooltip::measureText()
	{
		const glm::vec2 textSize = _typeface->layoutText(_text, resolvedFontSize(), _glyphs);
		node().intrinsicSize = textSize + glm::vec2{ kPadding * 2.0f, kPadding * 2.0f };
	}
}
