#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <optional>

namespace hl::ui
{
	class Checkbox : public Widget
	{
	public:
		explicit Checkbox(Node& node);

		void prepare() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		void setChecked(bool checked);
		bool checked() const { return _checked; }

		std::optional<glm::vec3> boxColor;
		std::optional<glm::vec3> checkColor;
		std::function<void(bool)> onChanged;

		glm::vec3 resolvedBoxColor() const { return resolve(boxColor, theme().well); }
		glm::vec3 resolvedCheckColor() const { return resolve(checkColor, theme().accent); }

	private:
		bool _checked = false;
	};
}
