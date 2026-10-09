#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <memory>

namespace hl::ui
{
	class ScrollView : public Widget
	{
	public:
		explicit ScrollView(Node& node);

		Node& content() { return *_viewport; }
		const Node& content() const { return *_viewport; }
		Node& viewport() { return *_viewport; }
		const Node& viewport() const { return *_viewport; }
		bool barVisible() const;

		void prepare() override;
		void afterLayout() override;

		glm::vec2 viewportSize{ 280.0f, 240.0f };
		bool scrollBars = true;
		glm::vec3 trackColor{ 0.25f, 0.28f, 0.32f };
		glm::vec3 thumbColor{ 0.95f, 0.95f, 0.97f };

	private:
		void layoutChrome();
		glm::vec2 resolvedViewportSize() const;

		Node* _viewport = nullptr;
		std::unique_ptr<Widget> _vBar;
	};
}
