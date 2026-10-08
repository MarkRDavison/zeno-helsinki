#include <helsinki/Ui/Layout/Layout.hpp>

namespace hl::ui
{
	void layout(Node& root, Box viewport)
	{
		root.measure();
		root.bakePinnedFromMeasure();
		root.arrange(viewport);
		root.clampScroll();
	}
}
