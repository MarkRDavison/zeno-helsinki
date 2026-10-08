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

		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float opacity = 1.0f;
		float borderWidth = 0.0f;
		glm::vec3 borderColor{ 0.08f, 0.09f, 0.12f };
		std::optional<NineSlice> nineSlice;
	};
}
