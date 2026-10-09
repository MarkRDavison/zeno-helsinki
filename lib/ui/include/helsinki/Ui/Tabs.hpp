#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
#include <optional>
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

		std::optional<float> maxHeaderWidth;
		std::optional<float> headerHeight;
		std::optional<glm::vec3> color;
		std::optional<glm::vec3> fillColor;
		std::optional<glm::vec3> hoverColor;
		std::optional<glm::vec3> selectedColor;
		std::function<void(int)> onChanged;

		float resolvedMaxHeaderWidth() const { return resolve(maxHeaderWidth, theme().controlWidth); }
		float resolvedHeaderHeight() const { return resolve(headerHeight, theme().controlHeight); }
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }
		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().surface); }
		glm::vec3 resolvedHoverColor() const { return resolve(hoverColor, theme().hover); }
		glm::vec3 resolvedSelectedColor() const { return resolve(selectedColor, theme().accent); }

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
