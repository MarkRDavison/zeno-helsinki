#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <optional>

namespace hl::ui
{
	class Toggle : public Widget
	{
	public:
		explicit Toggle(Node& node);

		void prepare() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		void setOn(bool on);
		bool isOn() const { return _on; }

		std::optional<glm::vec3> offColor;
		std::optional<glm::vec3> onColor;
		std::optional<glm::vec3> knobColor;
		std::function<void(bool)> onChanged;

		glm::vec3 resolvedOffColor() const { return resolve(offColor, theme().well); }
		glm::vec3 resolvedOnColor() const { return resolve(onColor, theme().accent); }
		glm::vec3 resolvedKnobColor() const { return resolve(knobColor, theme().foreground); }

	private:
		bool _on = false;
	};
}
