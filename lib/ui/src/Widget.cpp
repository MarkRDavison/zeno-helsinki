#include <helsinki/Ui/Widget.hpp>

namespace hl::ui
{
	namespace
	{
		Widget* gPointerCapture = nullptr;
	}

	Widget::Widget(Node& node) :
		_node(&node)
	{
		_node->_widget = this;
	}

	Widget::~Widget()
	{
		releasePointer();
		if (_node && _node->_widget == this)
		{
			_node->_widget = nullptr;
		}
	}

	void Widget::capturePointer()
	{
		gPointerCapture = this;
	}

	void Widget::releasePointer()
	{
		if (gPointerCapture == this)
		{
			gPointerCapture = nullptr;
		}
	}

	bool Widget::hasPointerCapture() const
	{
		return gPointerCapture == this;
	}

	Widget* Widget::parentWidget() const
	{
		if (!_node)
		{
			return nullptr;
		}

		for (Node* n = _node->parent(); n != nullptr; n = n->parent())
		{
			if (n->widget())
			{
				return n->widget();
			}
		}

		return nullptr;
	}

	Widget* hitTest(const Node& root, glm::vec2 position)
	{
		const auto& kids = root.children();
		for (auto it = kids.rbegin(); it != kids.rend(); ++it)
		{
			if (Widget* hit = hitTest(**it, position))
			{
				return hit;
			}
		}

		Widget* widget = root.widget();
		if (widget && widget->hitTestEnabled && root.world.contains(position))
		{
			return widget;
		}

		return nullptr;
	}

	void dispatch(Node& root, const Pointer& pointer)
	{
		Widget* start = gPointerCapture != nullptr
			? gPointerCapture
			: hitTest(root, pointer.position);

		for (Widget* widget = start; widget != nullptr; widget = widget->parentWidget())
		{
			if (widget->handle(pointer) == EventResult::Consume)
			{
				return;
			}
		}
	}

	void prepareTree(Node& root)
	{
		if (Widget* widget = root.widget())
		{
			widget->prepare();
		}

		for (const auto& child : root.children())
		{
			prepareTree(*child);
		}
	}

	void paintTree(const Node& root, IPaint& paint)
	{
		if (const Widget* widget = root.widget())
		{
			widget->paint(paint);
		}

		for (const auto& child : root.children())
		{
			paintTree(*child, paint);
		}
	}
}
