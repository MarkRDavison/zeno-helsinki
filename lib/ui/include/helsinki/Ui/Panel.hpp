#pragma once

#include <helsinki/Ui/Widget.hpp>

namespace hl::ui
{
	class Panel : public Widget
	{
	public:
		explicit Panel(Node& node);

		void paint(IPaint& paint) const override;

		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float opacity = 1.0f;
	};
}
