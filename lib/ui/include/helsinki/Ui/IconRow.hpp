#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <optional>

namespace hl::ui
{
	class IconRow : public Widget
	{
	public:
		explicit IconRow(Node& node);

		void setCount(int count);
		int count() const { return _count; }

		void prepare() override;
		void paint(IPaint& paint) const override;

		std::optional<glm::vec2> iconSize;
		std::optional<float> gap;
		glm::vec4 uvRect{ 0.0f, 0.0f, 1.0f, 1.0f };
		std::optional<glm::vec3> color;

		glm::vec2 resolvedIconSize() const
		{
			const float h = theme().controlHeight;
			return resolve(iconSize, glm::vec2{ h, h });
		}

		float resolvedGap() const { return resolve(gap, theme().gap); }
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }

	private:
		glm::vec2 contentSize() const;

		int _count = 0;
	};
}
