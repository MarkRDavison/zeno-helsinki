#include <helsinki/Ui/Panel.hpp>

namespace hl::ui
{
	Panel::Panel(Node& node) :
		Widget(node)
	{
	}

	void Panel::paint(IPaint& paint) const
	{
		const auto& box = node().world;
		if (nineSlice.has_value())
		{
			paint.nineSlice(box, *nineSlice);
			return;
		}

		if (borderWidth > 0.0f)
		{
			paint.fill(box, glm::vec4{ borderColor, opacity });
			const float inset = borderWidth * 2.0f;
			if (box.size.x > inset && box.size.y > inset)
			{
				paint.fill(
					Box{
						box.pos.x + borderWidth,
						box.pos.y + borderWidth,
						box.size.x - inset,
						box.size.y - inset
					},
					glm::vec4{ color, opacity });
			}

			return;
		}

		paint.fill(box, glm::vec4{ color, opacity });
	}
}
