#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hl::ui
{
	enum class RadioOrientation
	{
		Vertical,
		Horizontal
	};

	class RadioGroup : public Widget
	{
	public:
		RadioGroup(Node& node, const ITypeface& typeface);

		void setItems(std::vector<std::string> items);
		const std::vector<std::string>& items() const { return _items; }

		int selectedIndex() const { return _selected; }
		void setSelectedIndex(int index);

		RadioOrientation orientation() const { return _orientation; }
		void setOrientation(RadioOrientation orientation);

		void prepare() override;
		EventResult handleKey(TextKey key) override;

		void pick(int index);
		void setHoverIndex(int index);
		int hoverIndex() const { return _hover; }

		unsigned fontSize = 16;
		float markSize = 18.0f;
		float labelGap = 8.0f;
		glm::vec3 color{ 0.95f, 0.95f, 0.97f };
		glm::vec3 wellColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 hoverColor{ 0.32f, 0.34f, 0.42f };
		glm::vec3 selectedColor{ 1.0f, 0.5f, 0.0f };
		glm::vec3 highlightColor{ 1.0f, 0.5f, 0.0f };
		std::function<void(int)> onChanged;

	private:
		void rebuildOptions();
		void moveSelection(int delta);
		void applyOrientation();

		const ITypeface* _typeface = nullptr;
		std::vector<std::unique_ptr<Widget>> _options;
		std::vector<std::string> _items;
		RadioOrientation _orientation = RadioOrientation::Vertical;
		int _selected = 0;
		int _hover = -1;
	};
}
