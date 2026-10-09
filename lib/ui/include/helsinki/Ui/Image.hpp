#pragma once

#include <helsinki/Ui/Widget.hpp>

namespace hl::ui
{
	class Image : public Widget
	{
	public:
		explicit Image(Node& node);

		void prepare() override;
		void paint(IPaint& paint) const override;

		glm::vec2 size{ 64.0f, 64.0f };
		glm::vec4 uvRect{ 0.0f, 0.0f, 1.0f, 1.0f };
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
	};
}
