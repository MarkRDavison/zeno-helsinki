#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>

namespace hl::ui
{
	class Slider : public Widget
	{
	public:
		explicit Slider(Node& node);

		void prepare() override;
		EventResult handle(const Pointer& pointer) override;
		void paint(IPaint& paint) const override;

		void setValue(float value);
		float value() const { return _value; }

		glm::vec3 trackColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 fillColor{ 1.0f, 0.5f, 0.0f };
		glm::vec3 thumbColor{ 0.95f, 0.95f, 0.97f };
		std::function<void(float)> onChanged;

	private:
		void applyPointer(const Pointer& pointer);

		float _value = 0.0f;
	};
}
