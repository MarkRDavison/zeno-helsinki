#pragma once

#include <helsinki/Ui/Button.hpp>
#include <helsinki/Ui/Label.hpp>
#include <helsinki/Ui/Panel.hpp>
#include <helsinki/Ui/Widget.hpp>

#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hl::ui
{
	enum class SnackbarType
	{
		Success,
		Warning,
		Error,
		Info
	};

	enum class SnackbarCorner
	{
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	struct SnackbarItem
	{
		SnackbarType type = SnackbarType::Info;
		std::string title;
		std::string description;
		bool persistent = false;
		std::optional<float> duration;
		std::optional<float> fade;
	};

	class SnackbarHost : public Widget
	{
	public:
		SnackbarHost(Node& host, const ITypeface& typeface);
		~SnackbarHost() override;

		void show(SnackbarItem item);
		void tick(float dt);
		void requestClose(std::size_t oldestIndex);

		void prepare() override;
		void afterLayout() override;

		SnackbarCorner corner = SnackbarCorner::BottomRight;
		std::optional<int> maxVisible;

		int resolvedMaxVisible() const;
		float resolvedDuration() const { return theme().snackbarDuration; }
		float resolvedFade() const { return theme().snackbarFade; }

		std::size_t visibleCount() const { return _toasts.size(); }
		std::size_t queuedCount() const { return _queue.size(); }
		bool toastPersistent(std::size_t oldestIndex) const;
		SnackbarType toastType(std::size_t oldestIndex) const;
		float toastOpacity(std::size_t oldestIndex) const;
		float toastTimerWidth(std::size_t oldestIndex) const;
		const Node& toastNode(std::size_t oldestIndex) const;
		glm::vec3 typeFill(SnackbarType type) const;
		glm::vec3 typeColor(SnackbarType type) const;

	private:
		struct Toast
		{
			SnackbarItem item;
			float age = 0.0f;
			float fadeElapsed = 0.0f;
			bool fading = false;
			Node* node = nullptr;
			std::unique_ptr<Panel> panel;
			std::unique_ptr<Label> title;
			std::unique_ptr<Label> description;
			std::unique_ptr<Button> close;
			std::unique_ptr<Panel> timer;
		};

		Node* treeRoot();
		void attachLast();
		void insertToast(SnackbarItem item);
		void destroyToast(std::size_t index);
		void startFade(std::size_t index);
		void flushQueue();
		void syncToastVisuals(Toast& toast);
		float toastDuration(const Toast& toast) const;
		float toastFade(const Toast& toast) const;
		float timerRemaining(const Toast& toast) const;
		glm::vec2 measureToast(const Toast& toast) const;

		const ITypeface* _typeface = nullptr;
		std::vector<std::unique_ptr<Toast>> _toasts;
		std::deque<SnackbarItem> _queue;
	};
}
