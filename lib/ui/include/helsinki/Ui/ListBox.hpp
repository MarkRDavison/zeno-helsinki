#pragma once

#include <helsinki/Ui/ScrollView.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <memory>
#include <vector>

namespace hl::ui
{
	class ListBox : public Widget
	{
	public:
		explicit ListBox(Node& node);

		template<typename T, typename Make>
		void setItems(const std::vector<T>& items, Make&& make)
		{
			clearRows();
			for (const auto& item : items)
			{
				Node& row = _scroll->content().addChild();
				_rows.push_back(make(row, item));
			}
		}

		int itemCount() const { return static_cast<int>(_rows.size()); }
		ScrollView& scroll() { return *_scroll; }
		const ScrollView& scroll() const { return *_scroll; }

		void prepare() override;
		void afterLayout() override;

		glm::vec2 viewportSize{ 280.0f, 240.0f };
		bool scrollBars = true;

	private:
		void clearRows();
		void syncScroll();

		std::unique_ptr<ScrollView> _scroll;
		std::vector<std::unique_ptr<Widget>> _rows;
	};
}
