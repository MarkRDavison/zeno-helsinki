#include <helsinki/Ui/Dialog.hpp>

#include <algorithm>

namespace hl::ui
{
	namespace
	{
		Dialog* gOpenDialog = nullptr;

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
	}

	void dismissModalOnEscape()
	{
		if (gOpenDialog != nullptr && gOpenDialog->closeOnEscape)
		{
			gOpenDialog->setOpen(false);
		}
	}

	Dialog::Dialog(Node& host, const ITypeface& typeface) :
		Widget(rootOf(host).addChild()),
		_typeface(&typeface)
	{
		hitTestEnabled = true;
		focusable = true;
		visible = false;
		node().setFillParent();

		_card = &node().addChild();
		_card->kind = Kind::Absolute;
		_card->setCenter(resolvedCardSize());
		_cardPanel = std::make_unique<Panel>(*_card);
		_cardPanel->color = resolvedCardColor();
		_cardPanel->hitTestEnabled = true;

		_title = std::make_unique<Label>(_card->addChild(), typeface);
		_content = &_card->addChild();
		_content->kind = Kind::Column;
		_content->gap = theme().gap;

		_actionHost = &_card->addChild();
		_actionHost->kind = Kind::Row;
		_actionHost->gap = theme().gap;
		_actionHost->crossAlign = Align::End;
		_actionPanel = std::make_unique<Panel>(*_actionHost);
		_actionPanel->color = resolvedCardColor();
		_actionPanel->hitTestEnabled = false;

		_close = std::make_unique<Button>(node().addChild(), typeface);
		_close->setText("X");
		_close->onClick = [this]()
		{
			setOpen(false);
		};

		syncChrome();
	}

	Dialog::~Dialog()
	{
		if (gOpenDialog == this)
		{
			gOpenDialog = nullptr;
			setModalFocusRoot(nullptr);
		}
	}

	Node* Dialog::treeRoot()
	{
		return &rootOf(node());
	}

	void Dialog::setTitle(std::string title)
	{
		_titleText = std::move(title);
		if (_title)
		{
			_title->setText(_titleText);
			_title->visible = !_titleText.empty();
		}
	}

	Button& Dialog::addAction(std::string label, std::function<void()> onClick)
	{
		auto button = std::make_unique<Button>(_actionHost->addChild(), *_typeface);
		button->setText(std::move(label));
		button->onClick = std::move(onClick);
		Button& ref = *button;
		_actions.push_back(std::move(button));
		syncChrome();
		return ref;
	}

	void Dialog::clearActions()
	{
		_actions.clear();
		while (!_actionHost->children().empty())
		{
			_actionHost->releaseChild(*_actionHost->children().front());
		}

		syncChrome();
	}

	void Dialog::setOpen(bool open)
	{
		if (open)
		{
			dismissOpenOverlay(nullptr);
			if (gOpenDialog != nullptr && gOpenDialog != this)
			{
				gOpenDialog->setOpen(false);
			}

			_open = true;
			gOpenDialog = this;
			setModalFocusRoot(&node());
			attachOverlayLast();
			setFocused(true);
		}
		else
		{
			const bool wasOpen = _open;
			_open = false;
			if (gOpenDialog == this)
			{
				gOpenDialog = nullptr;
				setModalFocusRoot(nullptr);
			}

			if (hasKeyboardFocus())
			{
				releaseKeyboardFocus();
			}

			if (wasOpen && onClosed)
			{
				onClosed();
			}
		}

		syncChrome();
	}

	void Dialog::prepare()
	{
		syncChrome();
		_cardPanel->color = resolvedCardColor();
		_actionPanel->color = resolvedCardColor();
		node().setFillParent();
	}

	void Dialog::afterLayout()
	{
		if (!_open)
		{
			return;
		}

		attachOverlayLast();
		layoutChrome();
	}

	void Dialog::attachOverlayLast()
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

	void Dialog::syncChrome()
	{
		visible = _open;
		hitTestEnabled = _open;
		focusable = _open;
		if (_title)
		{
			_title->visible = _open && !_titleText.empty();
			_title->setText(_titleText);
		}

		if (_actionPanel)
		{
			_actionPanel->visible = _open && !_actions.empty();
		}

		if (_close)
		{
			_close->visible = _open && closeButtonVisible;
			_close->hitTestEnabled = _close->visible;
			_close->focusable = _close->visible;
		}
	}

	void Dialog::pinInCard(Node& child, glm::vec2 localPos, glm::vec2 size)
	{
		child.setTopLeft(size);
		child.relative = localPos;
		child.arrange(_card->world);
	}

	void Dialog::layoutChrome()
	{
		const float pad = theme().paddingLarge;
		const float bandGap = theme().gap;
		const glm::vec2 preferred = resolvedCardSize();
		const float topPad = closeButtonVisible ? pad + kCloseSize.y : pad;
		const bool showTitle = _title != nullptr && _title->visible;
		const bool showActions = _actionPanel != nullptr && _actionPanel->visible;

		glm::vec2 titleSize{ 0.0f, 0.0f };
		if (showTitle)
		{
			titleSize = _title->node().measure();
		}
		else if (_title)
		{
			_title->node().intrinsicSize = glm::vec2{ 0.0f, 0.0f };
		}

		glm::vec2 actionSize{ 0.0f, 0.0f };
		if (showActions)
		{
			_actionHost->kind = Kind::Row;
			_actionHost->intrinsicSize.reset();
			actionSize = _actionHost->measure();
		}
		else
		{
			_actionHost->kind = Kind::Absolute;
			_actionHost->intrinsicSize = glm::vec2{ 0.0f, 0.0f };
		}

		_content->kind = Kind::Column;
		_content->intrinsicSize.reset();
		const glm::vec2 contentNatural = _content->measure();

		float gaps = 0.0f;
		if (showTitle)
		{
			gaps += bandGap;
		}

		if (showActions)
		{
			gaps += bandGap;
		}

		const float chromeY = topPad + pad + titleSize.y + actionSize.y + gaps;
		const float minContentH = std::max(0.0f, preferred.y - chromeY);
		const float contentH = std::max(minContentH, contentNatural.y);
		const float innerW = std::max({
			24.0f,
			preferred.x - pad * 2.0f,
			contentNatural.x,
			titleSize.x,
			actionSize.x
		});
		const float cardW = innerW + pad * 2.0f;
		const float cardH = chromeY + contentH;

		_card->kind = Kind::Absolute;
		_card->setCenter({ cardW, cardH });
		_card->arrange(node().world);

		float y = topPad;
		if (showTitle)
		{
			pinInCard(_title->node(), { pad, y }, titleSize);
			y += titleSize.y + bandGap;
		}
		else if (_title)
		{
			pinInCard(_title->node(), { pad, y }, { 0.0f, 0.0f });
		}

		pinInCard(*_content, { pad, y }, { innerW, contentH });
		y += contentH;

		if (showActions)
		{
			y += bandGap;
			pinInCard(*_actionHost, { pad, y }, { innerW, actionSize.y });
		}
		else
		{
			pinInCard(*_actionHost, { pad, y }, { 0.0f, 0.0f });
		}

		if (_close && _close->visible)
		{
			_close->node().setTopLeft(kCloseSize);
			_close->node().relative = _card->world.pos - node().world.pos
				+ glm::vec2{ _card->world.size.x - kCloseSize.x - 8.0f, 8.0f };
			_close->node().arrange(node().world);
		}
	}

	bool Dialog::pointOnCard(glm::vec2 position) const
	{
		return _card != nullptr && _card->world.contains(position);
	}

	EventResult Dialog::handle(const Pointer& pointer)
	{
		if (!_open)
		{
			return EventResult::Ignore;
		}

		if (pointer.primaryReleased && closeOnScrim && !pointOnCard(pointer.position))
		{
			setOpen(false);
		}

		return EventResult::Consume;
	}

	EventResult Dialog::handleKey(TextKey key)
	{
		if (!_open)
		{
			return EventResult::Ignore;
		}

		if (key == TextKey::Escape && closeOnEscape)
		{
			setOpen(false);
			return EventResult::Consume;
		}

		return EventResult::Ignore;
	}

	void Dialog::paint(IPaint& paint) const
	{
		if (!_open)
		{
			return;
		}

		paint.fill(node().world, resolvedScrimColor());
	}
}
