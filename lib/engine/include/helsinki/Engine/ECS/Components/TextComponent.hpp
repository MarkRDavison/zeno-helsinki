#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>

namespace hl
{

	class TextComponent : public Component
	{
	public:

		void setString(TextSystem& textSystem, const std::string& text, const std::string& font, unsigned size) 
		{
			_textSystemId = textSystem.registerText(_textSystemId, text, font, size);
			_text = text; 
			_font = font;
		}
		const std::string& getString() const
		{
			return _text;
		}
		const std::string& getFont() const
		{
			return _font;
		}

		int getTextSystemId() const { return _textSystemId; };

		glm::vec4 getColour() const { return _colour; }
		void setColour(const glm::vec4& colour) { _colour = colour; }

	private:
		int _textSystemId{ -1 };
		std::string _text;
		std::string _font;
		glm::vec4 _colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	};
}