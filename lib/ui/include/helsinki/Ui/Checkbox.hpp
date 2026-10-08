#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>

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

		glm::vec3 boxColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 checkColor{ 1.0f, 0.5f, 0.0f };
		std::function<void(bool)> onChanged;

	private:
		bool _checked = false;
	};
}
