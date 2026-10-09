#include <helsinki/Ui/Label.hpp>

namespace hl::ui
{
	Label::Label(Node& node, const ITypeface& typeface) :
		Widget(node),
		_typeface(&typeface),
		_drawColor(color)
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
		_drawColor = color;
		const glm::vec2 size = _typeface->layoutText(_text, resolvedFontSize(), _glyphs);
		node().intrinsicSize = size;
	}

	void Label::paint(IPaint& paint) const
	{
		paint.glyphs(_glyphs, node().world.pos, _drawColor);
	}
}
