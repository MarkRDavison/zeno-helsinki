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

		const float width = resolvedBorderWidth();
		const glm::vec3 fill = resolvedColor();
		if (width > 0.0f)
		{
			paint.fill(box, glm::vec4{ resolvedBorderColor(), opacity });
			const float inset = width * 2.0f;
			if (box.size.x > inset && box.size.y > inset)
			{
				paint.fill(
					Box{
						box.pos.x + width,
						box.pos.y + width,
						box.size.x - inset,
						box.size.y - inset
					},
					glm::vec4{ fill, opacity });
			}

			return;
		}

		paint.fill(box, glm::vec4{ fill, opacity });
	}
}
