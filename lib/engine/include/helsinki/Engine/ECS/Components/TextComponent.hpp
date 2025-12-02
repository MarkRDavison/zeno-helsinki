#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>

namespace hl
{

	class TextComponent : public Component
	{
	public:

		void setString(TextSystem& textSystem, const std::string& text, const std::string& font) 
		{
			_textSystemId = textSystem.registerText(_textSystemId, text, font);
			_text = text; 
		}
		const std::string& getString() const 
		{ 
			return _text;
		}

		int getTextSystemId() const { return _textSystemId; };

	private:
		int _textSystemId{ -1 };
		std::string _text;
	};
}