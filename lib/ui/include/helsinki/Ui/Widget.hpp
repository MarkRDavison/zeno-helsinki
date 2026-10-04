#pragma once

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Ui/Layout/Node.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Pointer.hpp>

namespace hl::ui
{
	class Widget : public NonCopyable
	{
	public:
		explicit Widget(Node& node);
		~Widget() override;

		Node& node() { return *_node; }
		const Node& node() const { return *_node; }
		Widget* parentWidget() const;

		virtual void prepare() {}
		virtual EventResult handle(const Pointer&) { return EventResult::Ignore; }
		virtual void paint(IPaint&) const {}

		void capturePointer();
		void releasePointer();
		bool hasPointerCapture() const;

		bool hitTestEnabled = true;

	private:
		Node* _node = nullptr;
	};

	Widget* hitTest(const Node& root, glm::vec2 position);
	void dispatch(Node& root, const Pointer& pointer);
	void prepareTree(Node& root);
	void paintTree(const Node& root, IPaint& paint);
}
