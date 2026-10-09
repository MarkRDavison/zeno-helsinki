#pragma once

#include <helsinki/Ui/Widget.hpp>

#include <memory>
#include <optional>

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

		std::optional<glm::vec2> viewportSize;
		bool scrollBars = true;
		std::optional<glm::vec3> trackColor;
		std::optional<glm::vec3> thumbColor;

		glm::vec2 resolvedViewportSize() const;
		glm::vec3 resolvedTrackColor() const { return resolve(trackColor, theme().well); }
		glm::vec3 resolvedThumbColor() const { return resolve(thumbColor, theme().foreground); }

	private:
		void layoutChrome();

		Node* _viewport = nullptr;
		std::unique_ptr<Widget> _vBar;
	};
}
