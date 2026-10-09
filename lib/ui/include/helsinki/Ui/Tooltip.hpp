#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	class Tooltip : public Widget
	{
	public:
		Tooltip(Node& overlayNode, const ITypeface& typeface);
		~Tooltip() override;

		void tick(float dt);
		void syncFromHit(Widget* hit);
		bool isVisible() const { return _visible; }
		float resolvedDelay() const;
		float resolvedOffset() const;
		TooltipPlacement resolvedPlacement() const;

		void prepare() override;
		void afterLayout() override;
		void paint(IPaint& paint) const override;

		std::optional<float> delay;
		std::optional<float> offset;
		std::optional<TooltipPlacement> placement;
		std::optional<glm::vec3> fillColor;
		std::optional<glm::vec3> color;

		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().background); }
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }

	private:
		Node* treeRoot();
		void attachLast();
		void show(Widget* source);
		void hide();
		void measureText();

		const ITypeface* _typeface = nullptr;
		Widget* _pending = nullptr;
		Widget* _hoverSource = nullptr;
		Widget* _source = nullptr;
		std::string _text;
		std::vector<GlyphVertex> _glyphs;
		float _accum = 0.0f;
		bool _visible = false;
	};
}
