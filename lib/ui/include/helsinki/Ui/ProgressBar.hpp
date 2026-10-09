#pragma once

#include <helsinki/Ui/Widget.hpp>

namespace hl::ui
{
	class ProgressBar : public Widget
	{
	public:
		explicit ProgressBar(Node& node);

		void prepare() override;
		void paint(IPaint& paint) const override;

		void setValue(float value);
		float value() const { return _value; }

		glm::vec3 trackColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 fillColor{ 1.0f, 0.5f, 0.0f };

	private:
		float _value = 0.0f;
	};
}
