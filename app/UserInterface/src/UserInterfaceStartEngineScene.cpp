#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/System/Events/CharEvent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/ScrollEvent.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <GLFW/glfw3.h>
#include <string>

namespace ui
{
	namespace
	{
		constexpr float kTexWhite = 0.0f;
		constexpr float kScrollPixels = 32.0f;

		class BatchPaint : public hl::ui::IPaint
		{
		public:
			explicit BatchPaint(hl::UiBatch& batch) : _batch(&batch) {}

			void fill(const hl::ui::Box& box, glm::vec4 color) override
			{
				_batch->addQuad(box, color, glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f }, kTexWhite);
			}

			void sprite(const hl::ui::Box&, glm::vec4, glm::vec3) override {}

			void glyphs(const std::vector<hl::ui::GlyphVertex>&, glm::vec2, glm::vec3) override {}

			void pushClip(const hl::ui::Box& worldBox) override
			{
				_batch->pushClip(worldBox);
			}

			void popClip() override
			{
				_batch->popClip();
			}

		private:
			hl::UiBatch* _batch = nullptr;
		};

		hl::ui::Panel& addRow(
			std::vector<std::unique_ptr<hl::ui::Widget>>& widgets,
			hl::ui::Node& parent,
			glm::vec2 size,
			glm::vec3 color)
		{
			auto row = std::make_unique<hl::ui::Panel>(parent.addChild());
			row->node().intrinsicSize = size;
			row->color = color;
			auto& ref = *row;
			widgets.push_back(std::move(row));
			return ref;
		}

		hl::ui::Pointer readPointer(hl::Engine& engine)
		{
			const auto window = engine.getInputManager().getWindowSize();
			const auto fb = engine.getInputManager().getFramebufferSize();
			auto mouse = engine.getInputManager().getMousePosition();
			if (window.x > 0.0f && window.y > 0.0f)
			{
				mouse.x *= fb.x / window.x;
				mouse.y *= fb.y / window.y;
			}

			return hl::ui::Pointer
			{
				.position = mouse,
				.primaryDown = engine.getInputManager().isButtonDown(GLFW_MOUSE_BUTTON_1),
				.primaryReleased = engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1)
			};
		}
	}

	UserInterfaceStartEngineScene::UserInterfaceStartEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig
	) :
		EngineScene(engine),
		_engineConfig(engineConfig)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	UserInterfaceStartEngineScene::~UserInterfaceStartEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void UserInterfaceStartEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		std::vector<hl::RenderpassInfo> renderpasses =
		{
			hl::RenderpassInfo
			{
				.name = "ui_renderpass",
				.inputs = {},
				.outputs =
				{
					hl::ResourceInfo
					{
						.name = "ui_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB",
						.clear = VkClearValue{ .color = { 0.08f, 0.09f, 0.12f, 1.0f} }
					}
				},
				.pipelineGroups =
				{
					{
						hl::PipelineInfo
						{
							.name = "ui_pipeline",
							.shaderVert = std::string(hl::RendererShaderRoot) + "/ui.vert",
							.shaderFrag = std::string(hl::RendererShaderRoot) + "/ui.frag",
							.descriptorSets =
							{
								hl::DescriptorSetInfo
								{
									.bindings =
									{
										hl::DescriptorBinding
										{
											.binding = 0,
											.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
											.stage = "VERTEX",
											.resource = cameraMatrixResourceId,
											.count = MAX_CAMERAS,
											.updateFrequency = hl::DescriptorUpdateFrequency::Static
										},
										hl::DescriptorBinding
										{
											.binding = 1,
											.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
											.stage = "FRAGMENT",
											.resource = "ui_sheet",
											.count = static_cast<uint32_t>(MAX_UI_TEXTURES),
											.updateFrequency = hl::DescriptorUpdateFrequency::Static,
											.partiallyBound = true,
											.updateAfterBind = true
										}
									}
								}
							},
							.vertexInputInfo = hl::VertexInputInfo
							{
								.attributes =
								{
									{
										.name = "inPosition",
										.format = hl::VertexAttributeFormat::Vec2,
										.location = 0,
										.offset = offsetof(hl::VertexUi2, pos)
									},
									{
										.name = "inColor",
										.format = hl::VertexAttributeFormat::Vec4,
										.location = 1,
										.offset = offsetof(hl::VertexUi2, color)
									},
									{
										.name = "inTexCoord",
										.format = hl::VertexAttributeFormat::Vec2,
										.location = 2,
										.offset = offsetof(hl::VertexUi2, texCoord)
									},
									{
										.name = "inTexIndex",
										.format = hl::VertexAttributeFormat::Float,
										.location = 3,
										.offset = offsetof(hl::VertexUi2, texIndex)
									}
								},
								.stride = sizeof(hl::VertexUi2)
							},
							.depthState =
							{
								.testEnable = false,
								.writeEnable = false
							},
							.rasterState =
							{
								.cullMode = VK_CULL_MODE_NONE
							},
							.enableBlending = true,
						}
					}
				}
			}
		};

		hl::ResourceContext resourceContext
		{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};

		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);

		const hl::ResourceDefinition uiSheetDefinition
		{
			.name = "ui_sheet",
			.type = "logical",
			.resources =
			{
				hl::ResourceDefinition::Child
				{
					.name = "white",
					.type = "texture"
				}
			}
		};

		resourceManager.LoadLogical(uiSheetDefinition, [&](const hl::ResourceDefinition::Child& child)
			{
				if (child.type != "texture")
				{
					return false;
				}

				resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
					child.name,
					resourceContext);

				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);

		buildLayoutTree();
		_uiBatch.initialise(device);

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiBatch.draw(pdd); });
	}

	void UserInterfaceStartEngineScene::buildLayoutTree()
	{
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();
		_widgets.clear();

		auto stretch = std::make_unique<hl::ui::Panel>(_layoutRoot->addChild());
		stretch->node().anchorMin = { 0.0f, 0.0f };
		stretch->node().anchorMax = { 1.0f, 0.0f };
		stretch->node().offset.left = 16.0f;
		stretch->node().offset.right = -16.0f;
		stretch->node().offset.top = 16.0f;
		stretch->node().offset.bottom = 64.0f;
		stretch->color = { 0.88f, 0.48f, 0.22f };
		_widgets.push_back(std::move(stretch));

		auto& topLeftNode = _layoutRoot->addChild();
		auto topLeft = std::make_unique<hl::ui::Panel>(topLeftNode);
		topLeft->node().setTopLeft({ 120.0f, 80.0f });
		topLeft->node().relative = { 16.0f, 88.0f };
		topLeft->color = { 0.32f, 0.55f, 0.86f };
		_widgets.push_back(std::move(topLeft));

		auto nested = std::make_unique<hl::ui::Panel>(topLeftNode.addChild());
		nested->node().setTopLeft({ 48.0f, 28.0f });
		nested->node().relative = { 10.0f, 10.0f };
		nested->color = { 0.95f, 0.88f, 0.35f };
		_widgets.push_back(std::move(nested));

		auto& column = _layoutRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 16.0f;
		column.padding = hl::ui::Edges::all(16.0f);
		column.crossAlign = hl::ui::Align::Start;
		column.setCenter({ 0.0f, 0.0f });

		auto card = std::make_unique<hl::ui::Panel>(column);
		card->color = { 0.16f, 0.17f, 0.22f };
		_widgets.push_back(std::move(card));

		_slider = std::make_unique<hl::ui::Slider>(column.addChild());
		_checkbox = std::make_unique<hl::ui::Checkbox>(column.addChild());
		_toggle = std::make_unique<hl::ui::Toggle>(column.addChild());

		auto& charStrip = _layoutRoot->addChild();
		charStrip.kind = hl::ui::Kind::Row;
		charStrip.gap = 4.0f;
		charStrip.setTopLeft({ 0.0f, 0.0f });
		charStrip.relative = { 16.0f, 176.0f };

		_charSlots.clear();
		_typedCodepoints.clear();
		for (int i = 0; i < 8; ++i)
		{
			auto& slot = addRow(_widgets, charStrip, { 28.0f, 28.0f }, { 0.18f, 0.19f, 0.22f });
			_charSlots.push_back(&slot);
		}

		auto& repeat = addRow(_widgets, charStrip, { 28.0f, 28.0f }, { 0.20f, 0.20f, 0.22f });
		_repeatMarker = &repeat;

		auto& clipList = _layoutRoot->addChild();
		clipList.kind = hl::ui::Kind::Column;
		clipList.clip = true;
		clipList.gap = 8.0f;
		clipList.padding = hl::ui::Edges::all(8.0f);
		clipList.setTopRight({ 220.0f, 280.0f });
		clipList.relative = { -16.0f, 88.0f };

		auto clipPanel = std::make_unique<hl::ui::Panel>(clipList);
		clipPanel->color = { 0.12f, 0.14f, 0.18f };
		_widgets.push_back(std::move(clipPanel));

		const glm::vec3 rowColors[] = {
			{ 0.70f, 0.32f, 0.32f },
			{ 0.70f, 0.48f, 0.22f },
			{ 0.85f, 0.78f, 0.28f },
			{ 0.32f, 0.62f, 0.38f },
			{ 0.28f, 0.52f, 0.72f },
			{ 0.42f, 0.38f, 0.72f },
			{ 0.62f, 0.32f, 0.58f },
			{ 0.55f, 0.55f, 0.58f }
		};
		for (int i = 0; i < 8; ++i)
		{
			auto& row = addRow(_widgets, clipList, { 204.0f, 36.0f }, rowColors[i]);
			if (i == 2)
			{
				_clipHitRow = &row;
			}
		}

		auto& nestedOuter = _layoutRoot->addChild();
		nestedOuter.clip = true;
		nestedOuter.setTopRight({ 180.0f, 120.0f });
		nestedOuter.relative = { -16.0f, 384.0f };

		auto nestedPanel = std::make_unique<hl::ui::Panel>(nestedOuter);
		nestedPanel->color = { 0.10f, 0.12f, 0.16f };
		_widgets.push_back(std::move(nestedPanel));

		auto& nestedInner = nestedOuter.addChild();
		nestedInner.kind = hl::ui::Kind::Column;
		nestedInner.clip = true;
		nestedInner.gap = 6.0f;
		nestedInner.setTopLeft({ 168.0f, 80.0f });
		nestedInner.relative = { 6.0f, 6.0f };

		auto nestedInnerPanel = std::make_unique<hl::ui::Panel>(nestedInner);
		nestedInnerPanel->color = { 0.16f, 0.20f, 0.24f };
		_widgets.push_back(std::move(nestedInnerPanel));

		for (int i = 0; i < 6; ++i)
		{
			const float t = static_cast<float>(i) / 5.0f;
			addRow(_widgets, nestedInner, { 168.0f, 28.0f }, { 0.25f + t * 0.5f, 0.55f, 0.65f - t * 0.3f });
		}

		auto& unclippedFrame = _layoutRoot->addChild();
		unclippedFrame.setBottomRight({ 100.0f, 48.0f });
		unclippedFrame.relative = { -16.0f, -16.0f };

		auto unclippedPanel = std::make_unique<hl::ui::Panel>(unclippedFrame);
		unclippedPanel->color = { 0.18f, 0.12f, 0.12f };
		_widgets.push_back(std::move(unclippedPanel));

		auto& unclipped = unclippedFrame.addChild();
		unclipped.kind = hl::ui::Kind::Column;
		unclipped.gap = 4.0f;
		unclipped.setTopLeft({ 100.0f, 0.0f });

		addRow(_widgets, unclipped, { 100.0f, 28.0f }, { 0.90f, 0.40f, 0.40f });
		addRow(_widgets, unclipped, { 100.0f, 28.0f }, { 0.90f, 0.55f, 0.35f });
		addRow(_widgets, unclipped, { 100.0f, 28.0f }, { 0.90f, 0.70f, 0.30f });
	}

	void UserInterfaceStartEngineScene::rebuildAndDraw()
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto size = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, size.x, size.y });
		const auto pointer = readPointer(_engine);
		hl::ui::dispatch(*_layoutRoot, pointer);

		if (_clipHitRow != nullptr)
		{
			if (pointer.primaryReleased && hl::ui::hitTest(*_layoutRoot, pointer.position) == _clipHitRow)
			{
				_clipHitOn = !_clipHitOn;
			}

			_clipHitRow->color = _clipHitOn
				? glm::vec3{ 1.0f, 1.0f, 1.0f }
				: glm::vec3{ 0.85f, 0.78f, 0.28f };
		}

		for (std::size_t i = 0; i < _charSlots.size(); ++i)
		{
			if (i < _typedCodepoints.size())
			{
				const float t = static_cast<float>(_typedCodepoints[i] % 256) / 255.0f;
				_charSlots[i]->color = { t, 0.35f, 1.0f - t };
			}
			else
			{
				_charSlots[i]->color = { 0.18f, 0.19f, 0.22f };
			}
		}

		if (_repeatMarker != nullptr)
		{
			_repeatMarker->color = _keyRepeatLit
				? glm::vec3{ 0.95f, 0.85f, 0.30f }
				: glm::vec3{ 0.20f, 0.20f, 0.22f };
		}
		_keyRepeatLit = false;

		_uiBatch.setFullScissor(VkRect2D{
			{ 0, 0 },
			{ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) }
		});
		_uiBatch.begin();
		BatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void UserInterfaceStartEngineScene::OnEvent(const hl::Event& event)
	{
		if (_layoutRoot == nullptr)
		{
			return;
		}

		if (const auto* scroll = dynamic_cast<const hl::ScrollEvent*>(&event))
		{
			const auto pointer = readPointer(_engine);
			hl::ui::applyScroll(
				*_layoutRoot,
				pointer.position,
				{ 0.0f, -static_cast<float>(scroll->getY()) * kScrollPixels });
		}
		else if (const auto* typed = dynamic_cast<const hl::CharEvent*>(&event))
		{
			_typedCodepoints.push_back(typed->codepoint());
			if (_typedCodepoints.size() > _charSlots.size())
			{
				_typedCodepoints.erase(_typedCodepoints.begin());
			}
		}
		else if (dynamic_cast<const hl::KeyRepeatEvent*>(&event) != nullptr)
		{
			_keyRepeatLit = true;
		}
	}

	void UserInterfaceStartEngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
		rebuildAndDraw();
	}

	void UserInterfaceStartEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		_uiBatch.updateGpuResources(currentFrame);
	}

	void UserInterfaceStartEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}
}
