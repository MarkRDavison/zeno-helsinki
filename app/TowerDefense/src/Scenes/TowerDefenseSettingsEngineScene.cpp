#include "Scenes/TowerDefenseSettingsEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <format>
#include <helsinki/Ui/Widget.hpp>

namespace tower
{
	TowerDefenseSettingsEngineScene::TowerDefenseSettingsEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		hl::audio::Audio& audio
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_audio(audio)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	TowerDefenseSettingsEngineScene::~TowerDefenseSettingsEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	void TowerDefenseSettingsEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		hl::ResourceContext resourceContext
		{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};

		loadMenuUiSheet(
			resourceManager,
			resourceContext,
			"settings_ui_sheet",
			{
				hl::ResourceDefinition::Child{ .name = "white", .type = "texture" },
				hl::ResourceDefinition::Child{ .name = "roboto", .type = "texture" }
			});

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			{ makeMenuUiRenderpass(cameraMatrixResourceId, "settings_ui_sheet") });

		buildUi(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void TowerDefenseSettingsEngineScene::buildUi(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();

		_backdrop = std::make_unique<hl::ui::Panel>(_layoutRoot->addChild());
		_backdrop->node().setFillParent();
		_backdrop->color = { 0.06f, 0.07f, 0.09f };

		auto& stack = _layoutRoot->addChild();
		stack.kind = hl::ui::Kind::Column;
		stack.gap = 28.0f;
		stack.crossAlign = hl::ui::Align::Center;
		stack.setCenter({ 0.0f, 0.0f });

		_heading = std::make_unique<hl::ui::Label>(stack.addChild(), *_typeface);
		_heading->setText("Settings", 72);
		_heading->color = { 1.0f, 0.5f, 0.0f };

		auto& cardNode = stack.addChild();
		_card = std::make_unique<hl::ui::Panel>(cardNode);
		_card->color = { 0.14f, 0.16f, 0.20f };

		auto& body = cardNode.addChild();
		body.setFillParent();
		body.kind = hl::ui::Kind::Column;
		body.gap = 22.0f;
		body.padding = hl::ui::Edges::all(36.0f);
		body.crossAlign = hl::ui::Align::Start;

		auto addLabel = [&](hl::ui::Node& row, std::unique_ptr<hl::ui::Label>& label, const char* name)
		{
			label = std::make_unique<hl::ui::Label>(row.addChild(), *_typeface);
			label->setText(name, 36);
			label->color = { 0.85f, 0.88f, 0.92f };
		};

		const float controlGap = 24.0f;

		auto& volumeRow = body.addChild();
		volumeRow.kind = hl::ui::Kind::Row;
		volumeRow.gap = controlGap;
		volumeRow.crossAlign = hl::ui::Align::Center;
		addLabel(volumeRow, _volumeLabel, "Master Volume");
		_volume = std::make_unique<hl::ui::Slider>(volumeRow.addChild());
		_volume->setValue(_audio.masterVolume());
		_volume->onChanged = [this](float value)
		{
			_audio.setMasterVolume(value);
		};
		_volumePercent = std::make_unique<hl::ui::Label>(volumeRow.addChild(), *_typeface);
		_volumePercent->color = { 0.85f, 0.88f, 0.92f };

		auto& musicRow = body.addChild();
		musicRow.kind = hl::ui::Kind::Row;
		musicRow.gap = controlGap;
		musicRow.crossAlign = hl::ui::Align::Center;
		addLabel(musicRow, _musicLabel, "Music");
		_music = std::make_unique<hl::ui::Checkbox>(musicRow.addChild());
		_music->setChecked(!_audio.musicMuted());
		_music->onChanged = [this](bool checked)
		{
			_audio.setMusicMuted(!checked);
		};

		auto& fullscreenRow = body.addChild();
		fullscreenRow.kind = hl::ui::Kind::Row;
		fullscreenRow.gap = controlGap;
		fullscreenRow.crossAlign = hl::ui::Align::Center;
		addLabel(fullscreenRow, _fullscreenLabel, "Fullscreen");
		_fullscreen = std::make_unique<hl::ui::Toggle>(fullscreenRow.addChild());
		_fullscreen->setOn(_engine.isFullscreen());
		_fullscreen->onChanged = [this](bool on)
		{
			_engine.setFullscreen(on);
		};

		auto& vsyncRow = body.addChild();
		vsyncRow.kind = hl::ui::Kind::Row;
		vsyncRow.gap = controlGap;
		vsyncRow.crossAlign = hl::ui::Align::Center;
		addLabel(vsyncRow, _vsyncLabel, "VSync");
		_vsync = std::make_unique<hl::ui::Toggle>(vsyncRow.addChild());
		_vsync->setOn(_engine.isVsync());
		_vsync->onChanged = [this](bool on)
		{
			_engine.setVsync(on);
		};

		_volumeLabel->prepare();
		_musicLabel->prepare();
		_fullscreenLabel->prepare();
		_vsyncLabel->prepare();
		_volume->prepare();
		_music->prepare();
		_fullscreen->prepare();
		_vsync->prepare();

		const float labelColumnWidth = std::max({
			_volumeLabel->node().intrinsicSize->x,
			_musicLabel->node().intrinsicSize->x,
			_fullscreenLabel->node().intrinsicSize->x,
			_vsyncLabel->node().intrinsicSize->x
		});
		std::vector<hl::ui::GlyphVertex> unused;
		const float percentColumnWidth = _typeface->layoutText("100%", 32, unused).x;
		const float rowHeight = std::max({
			_volumeLabel->node().intrinsicSize->y,
			_volume->node().intrinsicSize->y,
			_music->node().intrinsicSize->y,
			_fullscreen->node().intrinsicSize->y,
			_vsync->node().intrinsicSize->y
		});
		const float cardWidth =
			labelColumnWidth + controlGap + _volume->node().intrinsicSize->x
			+ controlGap + percentColumnWidth
			+ body.padding.left + body.padding.right;
		const float cardHeight =
			rowHeight * 4.0f + body.gap * 3.0f
			+ body.padding.top + body.padding.bottom;
		cardNode.setCenter({ cardWidth, cardHeight });

		_back = std::make_unique<hl::ui::Button>(stack.addChild(), *_typeface);
		_back->setText("Back", 48);
		_back->onClick = [this]() { goBack(); };
	}

	void TowerDefenseSettingsEngineScene::goBack()
	{
		_sceneHost.goTitle();
	}

	void TowerDefenseSettingsEngineScene::rebuildAndDraw(float /*delta*/)
	{
		_volumePercent->setText(std::format("{:.0f}%", _volume->value() * 100.0f), 32);

		hl::ui::prepareTree(*_layoutRoot);

		hl::ui::Label* labels[] = {
			_volumeLabel.get(),
			_musicLabel.get(),
			_fullscreenLabel.get(),
			_vsyncLabel.get()
		};

		float labelColumnWidth = 0.0f;
		for (auto* label : labels)
		{
			if (label->node().intrinsicSize.has_value())
			{
				labelColumnWidth = std::max(labelColumnWidth, label->node().intrinsicSize->x);
			}
		}

		for (auto* label : labels)
		{
			if (!label->node().intrinsicSize.has_value())
			{
				continue;
			}

			label->node().intrinsicSize->x = labelColumnWidth;
		}

		std::vector<hl::ui::GlyphVertex> unused;
		const float percentColumnWidth = _typeface->layoutText("100%", 32, unused).x;
		if (_volumePercent->node().intrinsicSize.has_value())
		{
			_volumePercent->node().intrinsicSize->x = percentColumnWidth;
		}

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseSettingsEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		rebuildAndDraw(delta);
	}

	void TowerDefenseSettingsEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseSettingsEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}

	void TowerDefenseSettingsEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_ESCAPE)
			{
				goBack();
			}
		}
	}
}
