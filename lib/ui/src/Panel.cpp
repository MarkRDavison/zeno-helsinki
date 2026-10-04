#include <helsinki/Ui/Panel.hpp>

namespace hl::ui
{
	Panel::Panel(Node& node) :
		Widget(node)
	{
	}

	void Panel::paint(IPaint& paint) const
	{
		paint.fill(node().world, glm::vec4{ color, opacity });
	}
}
