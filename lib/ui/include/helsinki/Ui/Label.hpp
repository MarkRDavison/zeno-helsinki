#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <string>
#include <vector>

namespace hl::ui
{
	class Label : public Widget
	{
	public:
		Label(Node& node, const ITypeface& typeface);

		void setText(std::string text, unsigned fontSize);
		void prepare() override;
		void paint(IPaint& paint) const override;

		glm::vec3 color{ 1.0f, 1.0f, 1.0f };

	protected:
		glm::vec3 drawColor() const { return _drawColor; }
		void setDrawColor(glm::vec3 draw) { _drawColor = draw; }

	private:
		const ITypeface* _typeface = nullptr;
		std::string _text;
		unsigned _fontSize = 16;
		std::vector<GlyphVertex> _glyphs;
		glm::vec3 _drawColor{ 1.0f, 1.0f, 1.0f };
	};
}
