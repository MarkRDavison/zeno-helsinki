#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <optional>

namespace hl::ui
{
	class Panel : public Widget
	{
	public:
		explicit Panel(Node& node);

		void paint(IPaint& paint) const override;

		std::optional<glm::vec3> color;
		float opacity = 1.0f;
		std::optional<float> borderWidth;
		std::optional<glm::vec3> borderColor;
		std::optional<NineSlice> nineSlice;

		glm::vec3 resolvedColor() const { return resolve(color, theme().surface); }
		float resolvedBorderWidth() const { return resolve(borderWidth, theme().borderWidth); }
		glm::vec3 resolvedBorderColor() const { return resolve(borderColor, theme().border); }
	};
}
