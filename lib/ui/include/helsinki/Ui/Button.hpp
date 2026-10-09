#pragma once

#include <helsinki/Ui/Label.hpp>

#include <functional>
#include <optional>

namespace hl::ui
{
	enum class ButtonVariant
	{
		Default,
		Filled,
		Text
	};

	class Button : public Label
	{
	public:
		Button(Node& node, const ITypeface& typeface);

		void prepare() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		ButtonVariant variant = ButtonVariant::Default;
		std::optional<glm::vec3> hoverColor;
		std::optional<glm::vec3> fillColor;
		std::optional<glm::vec3> borderColor;
		std::optional<float> borderWidth;
		glm::vec3 resolvedHoverColor() const { return resolve(hoverColor, theme().accent); }
		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().background); }
		glm::vec3 resolvedBorderColor() const { return resolve(borderColor, theme().border); }
		float resolvedBorderWidth() const;
		std::function<void()> onClick;
		bool enabled{ true };

	private:
		bool chrome() const { return variant != ButtonVariant::Text; }
		void applyDrawColor();

		bool _hovered = false;
	};
}
