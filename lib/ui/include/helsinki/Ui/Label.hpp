#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	class Label : public Widget
	{
	public:
		Label(Node& node, const ITypeface& typeface);

		void setText(std::string text);
		void setText(std::string text, unsigned size);
		void prepare() override;
		void paint(IPaint& paint) const override;

		std::optional<glm::vec3> color;
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }

	protected:
		glm::vec3 drawColor() const { return _drawColor; }
		void setDrawColor(glm::vec3 draw) { _drawColor = draw; }
		glm::vec2 textLayoutSize() const { return _layoutSize; }
		void paintGlyphs(IPaint& paint, glm::vec2 origin) const;

	private:
		const ITypeface* _typeface = nullptr;
		std::string _text;
		std::vector<GlyphVertex> _glyphs;
		glm::vec2 _layoutSize{ 0.0f, 0.0f };
		glm::vec3 _drawColor{ 1.0f, 1.0f, 1.0f };
	};
}
