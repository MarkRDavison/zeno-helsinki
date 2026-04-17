#pragma once

#include <helsinki/UserInterface/IWidget.hpp>

namespace hl
{

	class UiScene;
	class TextWidget : public IWidget
	{
	public:
		void OnPaint(UiScene& a_Scene, DrawList& a_DrawList) override;

		void setText(UiScene& a_Scene, const std::string& text, const std::string& font);

	private:
		std::string _text;
		std::string _font;

		bool _autoTruncate{ true };
		std::vector<Vec4f> _generatedVertices;
		Rectf _generatedBounds;
	};

}