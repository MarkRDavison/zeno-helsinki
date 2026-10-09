#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	class Dropdown : public Widget
	{
	public:
		Dropdown(Node& header, const ITypeface& typeface);
		~Dropdown() override;

		void setItems(std::vector<std::string> items);
		const std::vector<std::string>& items() const { return _items; }

		int selectedIndex() const { return _selected; }
		void setSelectedIndex(int index);

		bool isOpen() const { return _open; }
		void setOpen(bool open);

		Node& overlay() { return *_overlay; }
		const Node& overlay() const { return *_overlay; }

		void setFocused(bool focused) override;
		void prepare() override;
		void afterLayout() override;
		EventResult handle(const Pointer& pointer) override;
		EventResult handleKey(TextKey key) override;
		void paint(IPaint& paint) const override;

		void pick(int index);
		void setHighlight(int index);

		std::optional<float> maxListHeight;
		std::optional<float> itemHeight;
		std::optional<glm::vec3> color;
		std::optional<glm::vec3> fillColor;
		std::optional<glm::vec3> hoverColor;
		std::optional<glm::vec3> highlightColor;
		std::function<void(int)> onChanged;

		float resolvedMaxListHeight() const
		{
			return resolve(maxListHeight, 5.0f * theme().controlHeight);
		}

		float resolvedItemHeight() const { return resolve(itemHeight, theme().controlHeight); }
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }
		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().surface); }
		glm::vec3 resolvedHoverColor() const { return resolve(hoverColor, theme().hover); }
		glm::vec3 resolvedHighlightColor() const { return resolve(highlightColor, theme().accent); }

		int highlightIndex() const { return _highlight; }

	private:
		Node* treeRoot();
		void rebuildRows();
		void attachOverlayLast();
		void syncOverlayHitTest();
		float contentHeight() const;
		const std::string& selectedText() const;

		const ITypeface* _typeface = nullptr;
		Node* _overlay = nullptr;
		std::unique_ptr<Panel> _overlayPanel;
		std::vector<std::unique_ptr<Widget>> _rows;
		std::vector<std::string> _items;
		std::vector<GlyphVertex> _glyphs;
		int _selected = 0;
		int _highlight = 0;
		bool _open = false;
		bool _hovered = false;
	};
}
