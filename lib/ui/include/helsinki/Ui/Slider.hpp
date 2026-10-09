#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <optional>

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

		std::optional<glm::vec3> trackColor;
		std::optional<glm::vec3> fillColor;
		std::optional<glm::vec3> thumbColor;
		std::function<void(float)> onChanged;

		glm::vec3 resolvedTrackColor() const { return resolve(trackColor, theme().well); }
		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().accent); }
		glm::vec3 resolvedThumbColor() const { return resolve(thumbColor, theme().foreground); }

	private:
		void applyPointer(const Pointer& pointer);

		float _value = 0.0f;
	};
}
