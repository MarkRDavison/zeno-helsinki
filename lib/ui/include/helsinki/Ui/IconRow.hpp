#pragma once

#include <helsinki/Ui/Widget.hpp>

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

		glm::vec2 iconSize{ 32.0f, 32.0f };
		float gap = 8.0f;
		glm::vec4 uvRect{ 0.0f, 0.0f, 1.0f, 1.0f };
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };

	private:
		glm::vec2 contentSize() const;

		int _count = 0;
	};
}
