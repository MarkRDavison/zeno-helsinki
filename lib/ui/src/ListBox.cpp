#include <helsinki/Ui/ListBox.hpp>

namespace hl::ui
{
	ListBox::ListBox(Node& node) :
		Widget(node)
	{
		hitTestEnabled = false;
		focusable = false;
		node.kind = Kind::Absolute;
		_scroll = std::make_unique<ScrollView>(node.addChild());
	}

	void ListBox::prepare()
	{
		if (!node().intrinsicSize.has_value())
		{
			node().intrinsicSize = viewportSize;
		}

		syncScroll();
	}

	void ListBox::afterLayout()
	{
		syncScroll();
		_scroll->node().arrange(node().world);
		_scroll->afterLayout();
	}

	void ListBox::clearRows()
	{
		_rows.clear();
		Node& content = _scroll->content();
		while (!content.children().empty())
		{
			content.releaseChild(*content.children().front());
		}
	}

	void ListBox::syncScroll()
	{
		_scroll->viewportSize = viewportSize;
		_scroll->scrollBars = scrollBars;

		glm::vec2 size = viewportSize;
		if (node().world.size.x > size.x)
		{
			size.x = node().world.size.x;
		}

		if (node().world.size.y > size.y)
		{
			size.y = node().world.size.y;
		}

		_scroll->node().setTopLeft(size);
		_scroll->node().relative = { 0.0f, 0.0f };
		_scroll->node().intrinsicSize = size;
		node().intrinsicSize = size;
	}
}
