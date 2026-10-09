#include <helsinki/Ui/Label.hpp>

namespace hl::ui
{
	Label::Label(Node& node, const ITypeface& typeface) :
		Widget(node),
		_typeface(&typeface),
		_drawColor(resolvedColor())
	{
		hitTestEnabled = false;
	}

	void Label::setText(std::string text)
	{
		_text = std::move(text);
	}

	void Label::setText(std::string text, unsigned size)
	{
		_text = std::move(text);
		fontSize = size;
	}

	void Label::prepare()
	{
		_drawColor = resolvedColor();
		_layoutSize = _typeface->layoutText(_text, resolvedFontSize(), _glyphs);
		node().intrinsicSize = _layoutSize;
	}

	void Label::paint(IPaint& paint) const
	{
		paintGlyphs(paint, node().world.pos);
	}

	void Label::paintGlyphs(IPaint& paint, glm::vec2 origin) const
	{
		paint.glyphs(_glyphs, origin, glm::vec4{ _drawColor, opacity });
	}
}
