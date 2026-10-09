#include <helsinki/Ui/Snackbar.hpp>

#include <algorithm>
#include <utility>

namespace hl::ui
{
	namespace
	{
		Node& rootOf(Node& host)
		{
			Node* root = &host;
			while (root->parent() != nullptr)
			{
				root = root->parent();
			}

			return *root;
		}

		constexpr glm::vec2 kCloseSize{ 28.0f, 24.0f };
		constexpr float kTimerHeight = 2.0f;
	}

	SnackbarHost::SnackbarHost(Node& host, const ITypeface& typeface) :
		Widget(rootOf(host).addChild()),
		_typeface(&typeface)
	{
		hitTestEnabled = false;
		focusable = false;
		node().setFillParent();
		attachLast();
	}

	SnackbarHost::~SnackbarHost() = default;

	int SnackbarHost::resolvedMaxVisible() const
	{
		return std::max(1, resolve(maxVisible, theme().snackbarMaxVisible));
	}

	glm::vec3 SnackbarHost::typeFill(SnackbarType type) const
	{
		switch (type)
		{
		case SnackbarType::Success:
			return theme().successMuted;
		case SnackbarType::Warning:
			return theme().warningMuted;
		case SnackbarType::Error:
			return theme().errorMuted;
		case SnackbarType::Info:
			return theme().infoMuted;
		}

		return theme().infoMuted;
	}

	glm::vec3 SnackbarHost::typeColor(SnackbarType type) const
	{
		switch (type)
		{
		case SnackbarType::Success:
			return theme().success;
		case SnackbarType::Warning:
			return theme().warning;
		case SnackbarType::Error:
			return theme().error;
		case SnackbarType::Info:
			return theme().info;
		}

		return theme().info;
	}

	bool SnackbarHost::toastPersistent(std::size_t oldestIndex) const
	{
		return _toasts.at(oldestIndex)->item.persistent;
	}

	SnackbarType SnackbarHost::toastType(std::size_t oldestIndex) const
	{
		return _toasts.at(oldestIndex)->item.type;
	}

	float SnackbarHost::toastOpacity(std::size_t oldestIndex) const
	{
		return _toasts.at(oldestIndex)->panel->opacity;
	}

	float SnackbarHost::toastTimerWidth(std::size_t oldestIndex) const
	{
		const Toast& toast = *_toasts.at(oldestIndex);
		if (toast.timer == nullptr || !toast.timer->visible)
		{
			return 0.0f;
		}

		return toast.timer->node().world.size.x;
	}

	const Node& SnackbarHost::toastNode(std::size_t oldestIndex) const
	{
		return *_toasts.at(oldestIndex)->node;
	}

	void SnackbarHost::show(SnackbarItem item)
	{
		if (static_cast<int>(_toasts.size()) < resolvedMaxVisible())
		{
			insertToast(std::move(item));
			return;
		}

		for (std::size_t i = 0; i < _toasts.size(); ++i)
		{
			if (!_toasts[i]->item.persistent)
			{
				destroyToast(i);
				insertToast(std::move(item));
				return;
			}
		}

		_queue.push_back(std::move(item));
	}

	void SnackbarHost::requestClose(std::size_t oldestIndex)
	{
		startFade(oldestIndex);
	}

	void SnackbarHost::tick(float dt)
	{
		for (std::size_t i = 0; i < _toasts.size();)
		{
			Toast& toast = *_toasts[i];
			if (toast.fading)
			{
				toast.fadeElapsed += dt;
				if (toast.fadeElapsed >= toastFade(toast))
				{
					destroyToast(i);
					continue;
				}
			}
			else if (!toast.item.persistent)
			{
				toast.age += dt;
				if (toast.age >= toastDuration(toast))
				{
					startFade(i);
				}
			}

			++i;
		}

		flushQueue();
	}

	void SnackbarHost::prepare()
	{
		attachLast();
		for (auto& toast : _toasts)
		{
			syncToastVisuals(*toast);
		}
	}

	void SnackbarHost::afterLayout()
	{
		Node* root = treeRoot();
		const auto& rootBox = root->world;
		const float inset = theme().paddingLarge;
		const float gap = theme().gap;
		const float pad = theme().padding;
		const bool fromBottom =
			corner == SnackbarCorner::BottomLeft || corner == SnackbarCorner::BottomRight;
		const bool fromRight =
			corner == SnackbarCorner::TopRight || corner == SnackbarCorner::BottomRight;

		auto layoutGroup = [&](bool persistent, bool stackFromBottom, bool stackFromRight, bool centerX)
		{
			float running = 0.0f;
			for (auto it = _toasts.rbegin(); it != _toasts.rend(); ++it)
			{
				Toast& toast = **it;
				if (toast.item.persistent != persistent)
				{
					continue;
				}

				const glm::vec2 size = measureToast(toast);
				float x = centerX
					? (rootBox.size.x - size.x) * 0.5f
					: (stackFromRight ? rootBox.size.x - inset - size.x : inset);
				float y = stackFromBottom
					? rootBox.size.y - inset - running - size.y
					: inset + running;

				toast.node->setTopLeft(size);
				toast.node->relative = { x, y };
				toast.node->arrange(rootBox);

				toast.title->node().setTopLeft(
					toast.title->node().intrinsicSize.value_or(glm::vec2{ 0.0f }));
				toast.title->node().relative = { pad, pad };
				toast.title->node().arrange(toast.node->world);

				const float titleHeight = toast.title->node().world.size.y;
				toast.description->node().setTopLeft(
					toast.description->node().intrinsicSize.value_or(glm::vec2{ 0.0f }));
				toast.description->node().relative = { pad, pad + titleHeight + gap };
				toast.description->node().arrange(toast.node->world);

				toast.close->node().setTopLeft(kCloseSize);
				toast.close->node().relative = { size.x - pad - kCloseSize.x, pad };
				toast.close->node().arrange(toast.node->world);

				if (toast.timer != nullptr && toast.timer->visible)
				{
					const float inner = std::max(0.0f, size.x - pad * 2.0f);
					const float barWidth = inner * timerRemaining(toast);
					toast.timer->node().setTopLeft({ barWidth, kTimerHeight });
					toast.timer->node().relative = { pad, size.y - pad - kTimerHeight };
					toast.timer->node().arrange(toast.node->world);
				}

				running += size.y + gap;
			}
		};

		layoutGroup(false, fromBottom, fromRight, false);
		layoutGroup(true, false, false, true);
	}

	Node* SnackbarHost::treeRoot()
	{
		Node* root = &node();
		while (root->parent() != nullptr)
		{
			root = root->parent();
		}

		return root;
	}

	void SnackbarHost::attachLast()
	{
		Node* root = treeRoot();
		if (node().parent() == root && !root->children().empty()
			&& root->children().back().get() == &node())
		{
			return;
		}

		if (node().parent() == nullptr)
		{
			return;
		}

		auto held = node().parent()->releaseChild(node());
		if (held)
		{
			root->addChild(std::move(held));
		}
	}

	void SnackbarHost::insertToast(SnackbarItem item)
	{
		auto toast = std::make_unique<Toast>();
		toast->item = std::move(item);
		toast->node = &node().addChild();
		toast->node->kind = Kind::Absolute;

		toast->panel = std::make_unique<Panel>(*toast->node);
		toast->panel->hitTestEnabled = true;

		toast->title = std::make_unique<Label>(toast->node->addChild(), *_typeface);
		toast->title->setText(toast->item.title, theme().fontSize + 2);

		toast->description = std::make_unique<Label>(toast->node->addChild(), *_typeface);
		toast->description->setText(toast->item.description);

		toast->timer = std::make_unique<Panel>(toast->node->addChild());
		toast->timer->hitTestEnabled = false;
		toast->timer->borderWidth = 0.0f;

		toast->close = std::make_unique<Button>(toast->node->addChild(), *_typeface);
		toast->close->variant = ButtonVariant::Text;
		toast->close->focusable = false;
		toast->close->setText("X");
		Toast* raw = toast.get();
		toast->close->onClick = [this, raw]()
		{
			for (std::size_t i = 0; i < _toasts.size(); ++i)
			{
				if (_toasts[i].get() == raw)
				{
					startFade(i);
					return;
				}
			}
		};

		syncToastVisuals(*toast);
		_toasts.push_back(std::move(toast));
		attachLast();
	}

	void SnackbarHost::destroyToast(std::size_t index)
	{
		Toast& toast = *_toasts.at(index);
		toast.close.reset();
		toast.timer.reset();
		toast.title.reset();
		toast.description.reset();
		toast.panel.reset();
		if (toast.node != nullptr && toast.node->parent() != nullptr)
		{
			toast.node->parent()->releaseChild(*toast.node);
			toast.node = nullptr;
		}

		_toasts.erase(_toasts.begin() + static_cast<std::ptrdiff_t>(index));
	}

	void SnackbarHost::startFade(std::size_t index)
	{
		Toast& toast = *_toasts.at(index);
		if (!toast.fading)
		{
			toast.fading = true;
			toast.fadeElapsed = 0.0f;
		}
	}

	void SnackbarHost::flushQueue()
	{
		while (!_queue.empty() && static_cast<int>(_toasts.size()) < resolvedMaxVisible())
		{
			SnackbarItem next = std::move(_queue.front());
			_queue.pop_front();
			insertToast(std::move(next));
		}
	}

	void SnackbarHost::syncToastVisuals(Toast& toast)
	{
		const float fade = toastFade(toast);
		float opacity = 1.0f;
		if (toast.fading && fade > 0.0f)
		{
			opacity = std::clamp(1.0f - toast.fadeElapsed / fade, 0.0f, 1.0f);
		}
		else if (toast.fading)
		{
			opacity = 0.0f;
		}

		toast.panel->color = typeFill(toast.item.type);
		toast.panel->borderColor = typeColor(toast.item.type);
		toast.panel->borderWidth = std::max(theme().borderWidth, 2.0f);
		toast.panel->opacity = opacity;
		toast.title->color = theme().background;
		toast.description->color = theme().well;
		toast.close->color = theme().background;
		toast.title->opacity = opacity;
		toast.description->opacity = opacity;
		toast.close->opacity = opacity;
		if (toast.timer)
		{
			toast.timer->color = typeColor(toast.item.type);
			toast.timer->opacity = opacity;
			toast.timer->visible = !toast.item.persistent && timerRemaining(toast) > 0.0f;
		}

		toast.title->setText(toast.item.title, theme().fontSize + 2);
		toast.description->setText(toast.item.description);
	}

	float SnackbarHost::toastDuration(const Toast& toast) const
	{
		return toast.item.duration.value_or(resolvedDuration());
	}

	float SnackbarHost::toastFade(const Toast& toast) const
	{
		return toast.item.fade.value_or(resolvedFade());
	}

	float SnackbarHost::timerRemaining(const Toast& toast) const
	{
		if (toast.item.persistent || toast.fading)
		{
			return 0.0f;
		}

		const float duration = toastDuration(toast);
		if (duration <= 0.0f)
		{
			return 0.0f;
		}

		return std::clamp(1.0f - toast.age / duration, 0.0f, 1.0f);
	}

	glm::vec2 SnackbarHost::measureToast(const Toast& toast) const
	{
		const float pad = theme().padding;
		const float gap = theme().gap;
		const glm::vec2 title = toast.title->node().intrinsicSize.value_or(glm::vec2{ 0.0f });
		const glm::vec2 desc = toast.description->node().intrinsicSize.value_or(glm::vec2{ 0.0f });
		float height = pad * 2.0f + title.y + gap + desc.y;
		if (!toast.item.persistent)
		{
			height += kTimerHeight;
		}

		return { theme().controlWidth, height };
	}
}
