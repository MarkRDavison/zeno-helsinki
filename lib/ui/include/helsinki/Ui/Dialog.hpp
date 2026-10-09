#pragma once

#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	class Dialog : public Widget
	{
	public:
		Dialog(Node& host, const ITypeface& typeface);
		~Dialog() override;

		void setOpen(bool open);
		bool isOpen() const { return _open; }

		void setTitle(std::string title);
		Node& content() { return *_content; }
		const Node& content() const { return *_content; }
		Node& overlay() { return node(); }
		const Node& overlay() const { return node(); }
		Node& card() { return *_card; }
		const Node& card() const { return *_card; }

		Button& addAction(std::string label, std::function<void()> onClick);
		void clearActions();

		void prepare() override;
		void afterLayout() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		bool closeOnScrim = true;
		bool closeOnEscape = true;
		bool closeButtonVisible = true;
		std::optional<glm::vec2> cardSize;
		std::optional<glm::vec4> scrimColor;
		std::optional<glm::vec3> cardColor;
		std::function<void()> onClosed;

		glm::vec2 resolvedCardSize() const { return resolve(cardSize, theme().viewportSize); }
		glm::vec4 resolvedScrimColor() const { return resolve(scrimColor, theme().scrim); }
		glm::vec3 resolvedCardColor() const { return resolve(cardColor, theme().surface); }

	private:
		Node* treeRoot();
		void attachOverlayLast();
		void syncChrome();
		void layoutChrome();
		void pinInCard(Node& child, glm::vec2 localPos, glm::vec2 size);
		bool pointOnCard(glm::vec2 position) const;

		const ITypeface* _typeface = nullptr;
		Node* _card = nullptr;
		Node* _content = nullptr;
		Node* _actionHost = nullptr;
		std::unique_ptr<Panel> _cardPanel;
		std::unique_ptr<Label> _title;
		std::unique_ptr<Panel> _actionPanel;
		std::unique_ptr<Button> _close;
		std::vector<std::unique_ptr<Button>> _actions;
		std::string _titleText;
		bool _open = false;
	};
}
