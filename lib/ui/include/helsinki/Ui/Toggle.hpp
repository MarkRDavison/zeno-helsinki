#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>

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

		glm::vec3 offColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 onColor{ 1.0f, 0.5f, 0.0f };
		glm::vec3 knobColor{ 0.95f, 0.95f, 0.97f };
		std::function<void(bool)> onChanged;

	private:
		bool _on = false;
	};
}
