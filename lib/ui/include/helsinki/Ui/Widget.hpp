#pragma once

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

	class Widget : public NonCopyable
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
		std::optional<unsigned> fontSize;
		std::string tooltip;

		unsigned resolvedFontSize() const;

	protected:
		void takeKeyboardFocus();
		void releaseKeyboardFocus();

	private:
		Node* _node = nullptr;
	};

	Widget* hitTest(const Node& root, glm::vec2 position);
	void dispatch(Node& root, const Pointer& pointer);
	void syncTooltip(Widget* hit);
	void dismissOpenOverlay(Widget* hit);
	void dispatchChar(uint32_t codepoint);
	void dispatchTextKey(Node& root, TextKey key);
	void collectFocusables(const Node& root, std::vector<Widget*>& out);
	void cycleFocus(Node& root, bool reverse);
	bool applyScroll(Node& root, glm::vec2 position, glm::vec2 delta);
	void prepareTree(Node& root);
	void paintTree(const Node& root, IPaint& paint);
}
