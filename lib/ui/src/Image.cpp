#include <helsinki/Ui/Image.hpp>

namespace hl::ui
{
	Image::Image(Node& node) :
		Widget(node)
	{
		hitTestEnabled = false;
		focusable = false;
		node.intrinsicSize = size;
	}

	void Image::prepare()
	{
		node().intrinsicSize = size;
	}

	void Image::paint(IPaint& paint) const
	{
		paint.sprite(node().world, uvRect, color);
	}
}
