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
		glm::vec3 fillColor{ 0.12f, 0.13f, 0.16f };
		glm::vec3 color{ 0.95f, 0.95f, 0.97f };

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
