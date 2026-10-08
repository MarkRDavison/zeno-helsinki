#pragma once

#include <helsinki/Ui/Label.hpp>

#include <functional>

namespace hl::ui
{
	class Button : public Label
	{
	public:
		Button(Node& node, const ITypeface& typeface);

		void prepare() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		glm::vec3 hoverColor{ 1.0f, 1.0f, 0.0f };
		std::function<void()> onClick;

	private:
		bool _hovered = false;
	};
}
