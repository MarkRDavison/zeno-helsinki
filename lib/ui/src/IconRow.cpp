#include <helsinki/Ui/IconRow.hpp>

#include <algorithm>

namespace hl::ui
{
	IconRow::IconRow(Node& node) :
		Widget(node)
	{
		hitTestEnabled = false;
	}

	void IconRow::setCount(int count)
	{
		_count = std::max(0, count);
	}

	glm::vec2 IconRow::contentSize() const
	{
		if (_count <= 0)
		{
			return { 0.0f, 0.0f };
		}

		const glm::vec2 size = resolvedIconSize();
		const float width = static_cast<float>(_count) * size.x
			+ static_cast<float>(_count - 1) * resolvedGap();
		return { width, size.y };
	}

	void IconRow::prepare()
	{
		node().intrinsicSize = contentSize();
	}

	void IconRow::paint(IPaint& paint) const
	{
		const Box& world = node().world;
		const glm::vec2 size = resolvedIconSize();
		const float g = resolvedGap();
		const glm::vec3 tint = resolvedColor();
		for (int i = 0; i < _count; ++i)
		{
			const float x = world.pos.x + static_cast<float>(i) * (size.x + g);
			paint.sprite(Box{ x, world.pos.y, size.x, size.y }, uvRect, tint);
		}
	}
}
