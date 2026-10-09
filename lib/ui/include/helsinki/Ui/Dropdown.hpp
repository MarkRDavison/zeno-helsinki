#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
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

		float maxListHeight = 140.0f;
		float itemHeight = 28.0f;
		glm::vec3 color{ 0.95f, 0.95f, 0.97f };
		glm::vec3 fillColor{ 0.18f, 0.19f, 0.24f };
		glm::vec3 hoverColor{ 0.32f, 0.34f, 0.42f };
		glm::vec3 highlightColor{ 1.0f, 0.5f, 0.0f };
		std::function<void(int)> onChanged;

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
