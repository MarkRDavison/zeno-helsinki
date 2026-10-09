#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hl::ui
{
	class Tabs : public Widget
	{
	public:
		Tabs(Node& node, const ITypeface& typeface);

		Node& addPage(std::string title);
		int pageCount() const { return static_cast<int>(_pages.size()); }
		Node& page(int index);

		int selectedIndex() const { return _selected; }
		void setSelectedIndex(int index);
		void pick(int index);

		Node& headerClip() { return *_clip; }
		const Node& headerClip() const { return *_clip; }
		bool chevronsVisible() const { return _overflow; }

		void prepare() override;
		void afterLayout() override;
		EventResult handleKey(TextKey key) override;

		void setHoverIndex(int index);
		int hoverIndex() const { return _hover; }
		void scrollHeaders(int direction);

		float maxHeaderWidth = 280.0f; // used when the Tabs node is not stretched
		float headerHeight = 28.0f;
		glm::vec3 color{ 0.95f, 0.95f, 0.97f };
		glm::vec3 fillColor{ 0.18f, 0.19f, 0.24f };
		glm::vec3 hoverColor{ 0.32f, 0.34f, 0.42f };
		glm::vec3 selectedColor{ 1.0f, 0.5f, 0.0f };
		std::function<void(int)> onChanged;

	private:
		void applyPageVisibility();
		void ensureSelectedVisible();
		void moveSelection(int delta);
		float headerContentWidth() const;
		float barWidth() const;
		void layoutHeaderStrip();
		void syncBodyAnchors();

		const ITypeface* _typeface = nullptr;
		Node* _bar = nullptr;
		Node* _clip = nullptr;
		Node* _body = nullptr;
		std::unique_ptr<Widget> _prev;
		std::unique_ptr<Widget> _next;
		std::vector<std::unique_ptr<Widget>> _headers;
		std::vector<std::unique_ptr<Widget>> _pageHosts;
		std::vector<Node*> _pages;
		std::vector<std::string> _titles;
		int _selected = 0;
		int _hover = -1;
		bool _overflow = false;
	};
}
