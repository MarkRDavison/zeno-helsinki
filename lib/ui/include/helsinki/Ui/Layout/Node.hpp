#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Ui/Layout/Box.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace hl::ui
{
	class Widget;

	class Node : public NonCopyable
	{
	public:
		Kind kind = Kind::Absolute;
		glm::vec2 anchorMin{ 0.0f, 0.0f };
		glm::vec2 anchorMax{ 0.0f, 0.0f };
		Edges offset{};
		glm::vec2 relative{ 0.0f, 0.0f };
		std::optional<glm::vec2> intrinsicSize;
		float gap = 0.0f;
		Edges padding{};
		Align crossAlign = Align::Start;
		bool clip = false;
		glm::vec2 scrollOffset{ 0.0f, 0.0f };

		Box local{};
		Box world{};

		Node() = default;
		~Node() override = default;

		Node* parent() const { return _parent; }
		Widget* widget() const { return _widget; }
		const std::vector<std::unique_ptr<Node>>& children() const { return _children; }

		Node& addChild(std::unique_ptr<Node> child);
		Node& addChild();
		std::unique_ptr<Node> releaseChild(Node& child);

		void setFillParent();
		void setPointAnchor(glm::vec2 anchor, glm::vec2 size, glm::vec2 pivot = { 0.5f, 0.5f });

		void setTopLeft(glm::vec2 size);
		void setTopCenter(glm::vec2 size);
		void setTopRight(glm::vec2 size);
		void setCenterLeft(glm::vec2 size);
		void setCenter(glm::vec2 size);
		void setCenterRight(glm::vec2 size);
		void setBottomLeft(glm::vec2 size);
		void setBottomCenter(glm::vec2 size);
		void setBottomRight(glm::vec2 size);

		glm::vec2 measure();
		void arrange(const Box& parentBox);
		glm::vec2 contentSize() const;
		glm::vec2 maxScroll() const;
		void clampScroll();

	private:
		friend class Widget;
		friend void layout(Node& root, Box viewport);

		void bakePinnedFromMeasure();
		void arrangeFromContainer();
		void arrangeChildren();
		void resolveAbsolute(const Box& parentBox);
		void packColumn();
		void packRow();

		Node* _parent = nullptr;
		Widget* _widget = nullptr;
		std::vector<std::unique_ptr<Node>> _children;
		glm::vec2 _cachedMeasure{ 0.0f };
	};

	void layout(Node& root, Box viewport);
}
