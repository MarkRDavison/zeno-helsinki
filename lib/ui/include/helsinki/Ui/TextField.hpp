#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <functional>
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
		bool focused() const { return _focused; }
		void setFocused(bool focused) override;

		EventResult handle(const Pointer& pointer) override;
		EventResult handleChar(uint32_t codepoint) override;
		EventResult handleKey(TextKey key) override;
		void prepare() override;
		void paint(IPaint& paint) const override;

		unsigned fontSize = 18;
		glm::vec3 color{ 0.95f, 0.95f, 0.97f };
		glm::vec3 background{ 0.12f, 0.13f, 0.16f };
		glm::vec3 caretColor{ 1.0f, 0.7f, 0.2f };
		std::function<void(std::string_view)> onChanged;

	private:
		void notifyChanged();
		float caretX() const;

		const ITypeface* _typeface = nullptr;
		std::string _text;
		std::size_t _caret = 0;
		bool _focused = false;
		float _padding = 8.0f;
		std::vector<GlyphVertex> _glyphs;
	};
}
