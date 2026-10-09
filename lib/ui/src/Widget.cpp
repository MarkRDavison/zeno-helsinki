#include <helsinki/Ui/Widget.hpp>

#include <cstddef>

namespace hl::ui
{
	namespace
	{
		Widget* gPointerCapture = nullptr;
		Widget* gKeyboardFocus = nullptr;
		const Node* gModalFocusRoot = nullptr;
	}

	void setModalFocusRoot(const Node* node)
	{
		gModalFocusRoot = node;
	}

	Widget::Widget(Node& node) :
		_node(&node)
	{
		_node->_widget = this;
	}

	Widget::~Widget()
	{
		releasePointer();
		releaseKeyboardFocus();
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

	bool Widget::hasKeyboardFocus() const
	{
		return gKeyboardFocus == this;
	}

	unsigned Widget::resolvedFontSize() const
	{
		return fontSize.value_or(theme().fontSize);
	}

	void Widget::setFocused(bool focused)
	{
		if (focused)
		{
			takeKeyboardFocus();
		}
		else
		{
			releaseKeyboardFocus();
		}
	}

	void Widget::takeKeyboardFocus()
	{
		if (gKeyboardFocus != nullptr && gKeyboardFocus != this)
		{
			gKeyboardFocus->setFocused(false);
		}

		gKeyboardFocus = this;
	}

	void Widget::releaseKeyboardFocus()
	{
		if (gKeyboardFocus == this)
		{
			gKeyboardFocus = nullptr;
		}
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
		if (Widget* widget = root.widget(); widget != nullptr && !widget->visible)
		{
			return nullptr;
		}

		if (root.clip && !root.world.contains(position))
		{
			return nullptr;
		}

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

	bool applyScroll(Node& root, glm::vec2 position, glm::vec2 delta)
	{
		if (Widget* widget = root.widget(); widget != nullptr && !widget->visible)
		{
			return false;
		}

		if (root.clip && !root.world.contains(position))
		{
			return false;
		}

		const auto& kids = root.children();
		for (auto it = kids.rbegin(); it != kids.rend(); ++it)
		{
			if (applyScroll(**it, position, delta))
			{
				return true;
			}
		}

		if (root.clip && root.world.contains(position))
		{
			root.scrollOffset += delta;
			root.clampScroll();
			return true;
		}

		return false;
	}

	void dispatch(Node& root, const Pointer& pointer)
	{
		Widget* start = gPointerCapture != nullptr
			? gPointerCapture
			: hitTest(root, pointer.position);

		if (pointer.primaryReleased && gKeyboardFocus != nullptr && gKeyboardFocus != start)
		{
			gKeyboardFocus->setFocused(false);
		}

		if (pointer.primaryReleased)
		{
			dismissOpenOverlay(start);
		}

		for (Widget* widget = start; widget != nullptr; widget = widget->parentWidget())
		{
			if (widget->handle(pointer) == EventResult::Consume)
			{
				break;
			}
		}

		syncTooltip(start);
	}

	void dispatchChar(uint32_t codepoint)
	{
		if (gKeyboardFocus != nullptr)
		{
			gKeyboardFocus->handleChar(codepoint);
		}
	}

	void collectFocusables(const Node& root, std::vector<Widget*>& out)
	{
		if (Widget* widget = root.widget())
		{
			if (!widget->visible)
			{
				return;
			}

			if (widget->focusable)
			{
				out.push_back(widget);
			}
		}

		for (const auto& child : root.children())
		{
			collectFocusables(*child, out);
		}
	}

	void cycleFocus(Node& root, bool reverse)
	{
		std::vector<Widget*> list;
		if (gModalFocusRoot != nullptr)
		{
			collectFocusables(*gModalFocusRoot, list);
		}
		else
		{
			collectFocusables(root, list);
		}
		if (list.empty())
		{
			return;
		}

		int current = -1;
		for (std::size_t i = 0; i < list.size(); ++i)
		{
			if (list[i]->hasKeyboardFocus())
			{
				current = static_cast<int>(i);
				break;
			}
		}

		int next = 0;
		if (reverse)
		{
			next = current <= 0 ? static_cast<int>(list.size()) - 1 : current - 1;
		}
		else
		{
			next = (current < 0 || current + 1 == static_cast<int>(list.size()))
				? 0
				: current + 1;
		}

		list[static_cast<std::size_t>(next)]->setFocused(true);
	}

	void dispatchTextKey(Node& root, TextKey key)
	{
		if (key == TextKey::Tab)
		{
			cycleFocus(root, false);
			return;
		}

		if (key == TextKey::ShiftTab)
		{
			cycleFocus(root, true);
			return;
		}

		if (gKeyboardFocus != nullptr)
		{
			if (gKeyboardFocus->handleKey(key) == EventResult::Consume)
			{
				return;
			}
		}

		if (key == TextKey::Escape)
		{
			dismissModalOnEscape();
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
		if (const Widget* widget = root.widget(); widget != nullptr && !widget->visible)
		{
			return;
		}

		if (const Widget* widget = root.widget())
		{
			widget->paint(paint);
		}

		if (root.clip)
		{
			paint.pushClip(root.world);
		}

		for (const auto& child : root.children())
		{
			paintTree(*child, paint);
		}

		if (root.clip)
		{
			paint.popClip();
		}
	}
}
