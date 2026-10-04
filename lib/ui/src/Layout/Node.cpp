#include <helsinki/Ui/Layout/Node.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

namespace hl::ui
{
	namespace
	{
		bool isPointPinnedX(const Node& node)
		{
			return node.anchorMin.x == node.anchorMax.x;
		}

		bool isPointPinnedY(const Node& node)
		{
			return node.anchorMin.y == node.anchorMax.y;
		}

		float crossOffset(float inner, float child, Align align)
		{
			switch (align)
			{
			case Align::Start:
				return 0.0f;
			case Align::Center:
				return (inner - child) * 0.5f;
			case Align::End:
				return inner - child;
			default:
				return 0.0f;
			}
		}

		void applyPointSizeX(Node& node, float width, float pivotX)
		{
			node.offset.left = -width * pivotX;
			node.offset.right = width * (1.0f - pivotX);
		}

		void applyPointSizeY(Node& node, float height, float pivotY)
		{
			node.offset.top = -height * pivotY;
			node.offset.bottom = height * (1.0f - pivotY);
		}
	}

	Node& Node::addChild(std::unique_ptr<Node> child)
	{
		child->_parent = this;
		Node& ref = *child;
		_children.push_back(std::move(child));
		return ref;
	}

	Node& Node::addChild()
	{
		return addChild(std::make_unique<Node>());
	}

	void Node::setFillParent()
	{
		anchorMin = { 0.0f, 0.0f };
		anchorMax = { 1.0f, 1.0f };
		offset = {};
		relative = { 0.0f, 0.0f };
	}

	void Node::setPointAnchor(glm::vec2 anchor, glm::vec2 size, glm::vec2 pivot)
	{
		anchorMin = anchor;
		anchorMax = anchor;
		applyPointSizeX(*this, size.x, pivot.x);
		applyPointSizeY(*this, size.y, pivot.y);
	}

	void Node::setTopLeft(glm::vec2 size)
	{
		setPointAnchor({ 0.0f, 0.0f }, size, { 0.0f, 0.0f });
	}

	void Node::setTopCenter(glm::vec2 size)
	{
		setPointAnchor({ 0.5f, 0.0f }, size, { 0.5f, 0.0f });
	}

	void Node::setTopRight(glm::vec2 size)
	{
		setPointAnchor({ 1.0f, 0.0f }, size, { 1.0f, 0.0f });
	}

	void Node::setCenterLeft(glm::vec2 size)
	{
		setPointAnchor({ 0.0f, 0.5f }, size, { 0.0f, 0.5f });
	}

	void Node::setCenter(glm::vec2 size)
	{
		setPointAnchor({ 0.5f, 0.5f }, size, { 0.5f, 0.5f });
	}

	void Node::setCenterRight(glm::vec2 size)
	{
		setPointAnchor({ 1.0f, 0.5f }, size, { 1.0f, 0.5f });
	}

	void Node::setBottomLeft(glm::vec2 size)
	{
		setPointAnchor({ 0.0f, 1.0f }, size, { 0.0f, 1.0f });
	}

	void Node::setBottomCenter(glm::vec2 size)
	{
		setPointAnchor({ 0.5f, 1.0f }, size, { 0.5f, 1.0f });
	}

	void Node::setBottomRight(glm::vec2 size)
	{
		setPointAnchor({ 1.0f, 1.0f }, size, { 1.0f, 1.0f });
	}

	glm::vec2 Node::measure()
	{
		if (kind == Kind::Column)
		{
			float maxWidth = 0.0f;
			float sumHeight = 0.0f;
			const auto n = _children.size();
			for (auto& child : _children)
			{
				const glm::vec2 childSize = child->measure();
				maxWidth = std::max(maxWidth, childSize.x);
				sumHeight += childSize.y;
			}
			const float gapTotal = n == 0 ? 0.0f : gap * static_cast<float>(n - 1);
			_cachedMeasure = {
				maxWidth + padding.left + padding.right,
				sumHeight + gapTotal + padding.top + padding.bottom
			};
			return _cachedMeasure;
		}

		if (kind == Kind::Row)
		{
			float sumWidth = 0.0f;
			float maxHeight = 0.0f;
			const auto n = _children.size();
			for (auto& child : _children)
			{
				const glm::vec2 childSize = child->measure();
				sumWidth += childSize.x;
				maxHeight = std::max(maxHeight, childSize.y);
			}
			const float gapTotal = n == 0 ? 0.0f : gap * static_cast<float>(n - 1);
			_cachedMeasure = {
				sumWidth + gapTotal + padding.left + padding.right,
				maxHeight + padding.top + padding.bottom
			};
			return _cachedMeasure;
		}

		for (auto& child : _children)
		{
			child->measure();
		}

		if (intrinsicSize.has_value())
		{
			_cachedMeasure = *intrinsicSize;
			return _cachedMeasure;
		}

		_cachedMeasure = {
			offset.right - offset.left,
			offset.bottom - offset.top
		};
		return _cachedMeasure;
	}

	void Node::bakePinnedFromMeasure()
	{
		const bool isContainer = kind != Kind::Absolute;
		const bool shouldBake = isContainer || intrinsicSize.has_value();
		if (shouldBake)
		{
			constexpr float pivot = 0.5f;
			if (isPointPinnedX(*this))
			{
				applyPointSizeX(*this, _cachedMeasure.x, pivot);
			}
			if (isPointPinnedY(*this))
			{
				applyPointSizeY(*this, _cachedMeasure.y, pivot);
			}
		}

		for (auto& child : _children)
		{
			child->bakePinnedFromMeasure();
		}
	}

	void Node::resolveAbsolute(const Box& parentBox)
	{
		const float left = parentBox.pos.x + parentBox.size.x * anchorMin.x + offset.left;
		const float top = parentBox.pos.y + parentBox.size.y * anchorMin.y + offset.top;
		const float right = parentBox.pos.x + parentBox.size.x * anchorMax.x + offset.right;
		const float bottom = parentBox.pos.y + parentBox.size.y * anchorMax.y + offset.bottom;

#ifdef HELSINKI_DEBUG
		assert(right >= left);
		assert(bottom >= top);
#endif

		local.pos = glm::vec2{ left - parentBox.pos.x, top - parentBox.pos.y } + relative;
		local.size = { right - left, bottom - top };
		world.pos = parentBox.pos + local.pos;
		world.size = local.size;
	}

	void Node::arrange(const Box& parentBox)
	{
		resolveAbsolute(parentBox);
		arrangeChildren();
	}

	void Node::arrangeFromContainer()
	{
		arrangeChildren();
	}

	void Node::arrangeChildren()
	{
		if (kind == Kind::Column)
		{
			packColumn();
			return;
		}
		if (kind == Kind::Row)
		{
			packRow();
			return;
		}

		for (auto& child : _children)
		{
			child->arrange(world);
		}
	}

	void Node::packColumn()
	{
		const float innerLeft = world.pos.x + padding.left;
		const float innerTop = world.pos.y + padding.top;
		const float innerWidth = world.size.x - padding.left - padding.right;
		float y = innerTop;

		for (auto& child : _children)
		{
			const glm::vec2 childSize = child->measure();
			const float x = innerLeft + crossOffset(innerWidth, childSize.x, crossAlign);
			child->local.pos = { x - world.pos.x, y - world.pos.y };
			child->local.size = childSize;
			child->world.pos = { x, y };
			child->world.size = childSize;
			child->arrangeFromContainer();
			y += childSize.y + gap;
		}
	}

	void Node::packRow()
	{
		const float innerLeft = world.pos.x + padding.left;
		const float innerTop = world.pos.y + padding.top;
		const float innerHeight = world.size.y - padding.top - padding.bottom;
		float x = innerLeft;

		for (auto& child : _children)
		{
			const glm::vec2 childSize = child->measure();
			const float y = innerTop + crossOffset(innerHeight, childSize.y, crossAlign);
			child->local.pos = { x - world.pos.x, y - world.pos.y };
			child->local.size = childSize;
			child->world.pos = { x, y };
			child->world.size = childSize;
			child->arrangeFromContainer();
			x += childSize.x + gap;
		}
	}

}
