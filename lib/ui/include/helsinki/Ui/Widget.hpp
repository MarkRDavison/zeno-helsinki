#pragma once

#include <helsinki/Ui/Export.hpp>

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <helsinki/Ui/Theme.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	enum class TextKey
	{
		Backspace,
		Delete,
		Left,
		Right,
		Home,
		End,
		Enter,
		Tab,
		ShiftTab,
		Space,
		Up,
		Down,
		Escape
	};

	class HELSINKI_UI_EXPORT Widget : public NonCopyable
	{
	public:
		explicit Widget(Node& node);
		~Widget() override;

		Node& node() { return *_node; }
		const Node& node() const { return *_node; }
		Widget* parentWidget() const;

		virtual void prepare() {}
		virtual void afterLayout() {}
		virtual EventResult handle(const Pointer&) { return EventResult::Ignore; }
		virtual EventResult handleChar(uint32_t) { return EventResult::Ignore; }
		virtual EventResult handleKey(TextKey) { return EventResult::Ignore; }
		virtual void setFocused(bool focused);
		virtual void paint(IPaint&) const {}

		void capturePointer();
		void releasePointer();
		bool hasPointerCapture() const;
		bool hasKeyboardFocus() const;

		bool hitTestEnabled = true;
		bool focusable = false;
		bool visible = true;
		std::optional<unsigned> fontSize;
		std::string tooltip;

		unsigned resolvedFontSize() const;

	protected:
		void takeKeyboardFocus();
		void releaseKeyboardFocus();

	private:
		Node* _node = nullptr;
	};

	HELSINKI_UI_EXPORT Widget* hitTest(const Node& root, glm::vec2 position);
	HELSINKI_UI_EXPORT void dispatch(Node& root, const Pointer& pointer);
	HELSINKI_UI_EXPORT void syncTooltip(Widget* hit);
	HELSINKI_UI_EXPORT void dismissOpenOverlay(Widget* hit);
	HELSINKI_UI_EXPORT void dismissModalOnEscape();
	HELSINKI_UI_EXPORT void setModalFocusRoot(const Node* node);
	HELSINKI_UI_EXPORT void dispatchChar(uint32_t codepoint);
	HELSINKI_UI_EXPORT void dispatchTextKey(Node& root, TextKey key);
	HELSINKI_UI_EXPORT void collectFocusables(const Node& root, std::vector<Widget*>& out);
	HELSINKI_UI_EXPORT void cycleFocus(Node& root, bool reverse);
	HELSINKI_UI_EXPORT bool applyScroll(Node& root, glm::vec2 position, glm::vec2 delta);
	HELSINKI_UI_EXPORT void prepareTree(Node& root);
	HELSINKI_UI_EXPORT void paintTree(const Node& root, IPaint& paint);
}
