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

		float markSize = 18.0f;
		std::optional<float> labelGap;
		std::optional<glm::vec3> color;
		std::optional<glm::vec3> wellColor;
		std::optional<glm::vec3> hoverColor;
		std::optional<glm::vec3> selectedColor;
		std::optional<glm::vec3> highlightColor;
		std::function<void(int)> onChanged;

		float resolvedLabelGap() const { return resolve(labelGap, theme().gap); }
		glm::vec3 resolvedColor() const { return resolve(color, theme().foreground); }
		glm::vec3 resolvedWellColor() const { return resolve(wellColor, theme().well); }
		glm::vec3 resolvedHoverColor() const { return resolve(hoverColor, theme().hover); }
		glm::vec3 resolvedSelectedColor() const { return resolve(selectedColor, theme().accent); }
		glm::vec3 resolvedHighlightColor() const { return resolve(highlightColor, theme().accent); }

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
