#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
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

		private:
			hl::UiBatch* _batch = nullptr;
		};

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
	}

	void UserInterfaceStartEngineScene::rebuildAndDraw()
	{
		hl::ui::prepareTree(*_layoutRoot);

		const auto size = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, size.x, size.y });
		hl::ui::dispatch(*_layoutRoot, readPointer(_engine));

		_uiBatch.begin();
		BatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
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
