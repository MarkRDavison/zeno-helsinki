#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hl::ui
{
	class TextField : public Widget
	{
	public:
		TextField(Node& node, const ITypeface& typeface);

		void setText(std::string text);
		const std::string& text() const { return _text; }
		bool focused() const { return hasKeyboardFocus(); }

		EventResult handle(const Pointer& pointer) override;
		EventResult handleChar(uint32_t codepoint) override;
		EventResult handleKey(TextKey key) override;
		void prepare() override;
		void paint(IPaint& paint) const override;

		std::optional<glm::vec3> color;
		std::optional<glm::vec3> background;
		std::optional<glm::vec3> caretColor;
		std::optional<float> padding;
		std::function<void(std::string_view)> onChanged;

		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }
		glm::vec3 resolvedBackground() const { return resolve(background, theme().background); }
		glm::vec3 resolvedCaretColor() const { return resolve(caretColor, theme().accent); }
		float resolvedPadding() const { return resolve(padding, theme().padding); }

	private:
		void notifyChanged();
		float caretX() const;
		float textHeight() const;
		float textOriginY(const Box& box) const;

		const ITypeface* _typeface = nullptr;
		std::string _text;
		std::size_t _caret = 0;
		glm::vec2 _textSize{ 0.0f, 0.0f };
		std::vector<GlyphVertex> _glyphs;
	};
}
