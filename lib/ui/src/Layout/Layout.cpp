#include <helsinki/Ui/Layout/Layout.hpp>
#include <helsinki/Ui/Widget.hpp>

namespace hl::ui
{
	namespace
	{
		void afterLayoutTree(Node& node)
		{
			if (Widget* widget = node.widget())
			{
				widget->afterLayout();
			}

			for (const auto& child : node.children())
			{
				afterLayoutTree(*child);
			}
		}
	}

	void layout(Node& root, Box viewport)
	{
		root.measure();
		root.bakePinnedFromMeasure();
		root.arrange(viewport);
		root.clampScroll();
		afterLayoutTree(root);
	}
}
