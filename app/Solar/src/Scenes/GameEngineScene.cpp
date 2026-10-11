#include <Scenes/GameEngineScene.hpp>
#include <GalaxyPushConstantObject.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <Scenes/TitleEngineScene.hpp>
#include <Views/GalaxyDrawConstants.hpp>
#include <World/GameCommand.hpp>
#include <World/PickFleet.hpp>
#include <World/PickStarSystem.hpp>
#include <World/StarterMap.hpp>
#include <World/StepGalaxy.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/ScrollEvent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <GLFW/glfw3.h>

namespace slr
{

	GameEngineScene::GameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		hl::imgui::Context& imgui)
		: EngineScene(engine)
		, _engineConfig(engineConfig)
		, _imgui(imgui)
		, _world(makeStarterMap())
	{
		_cameras.insert({ "Ui", new hl::Camera2D() });
		_galaxyCamera = new GalaxyCamera();
		_cameras.insert({ "Galaxy", _galaxyCamera });

		glm::vec2 sum{ 0.0f };
		for (const auto& system : _world.systems)
		{
			sum += system.position;
		}
		if (!_world.systems.empty())
		{
			_galaxyCamera->setOrigin(sum / static_cast<float>(_world.systems.size()));
		}

		_engine.getEventBus().AddListener(this);
		_imgui.setBuildUi(
			[this]
			{
				const TimeChromeResult time = _timeChrome.draw(_clock, _world.tick);
				if (time.speed.has_value())
				{
					_clock.setSpeed(time.speed.value());
				}
				if (time.paused.has_value())
				{
					_clock.setPaused(time.paused.value());
				}
				if (!_galaxy.drawInspector())
				{
					_galaxy.setSelectedSystemId(std::nullopt);
				}
			});
	}

	GameEngineScene::~GameEngineScene()
	{
		_imgui.setBuildUi({});
		_engine.getEventBus().RemoveListener(this);
	}

	void GameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		auto sceneRenderpassInfo = hl::RenderpassInfo{
			.name = "scene_pass",
			.inputs = {},
			.outputs =
			{
				hl::ResourceInfo{
					.name = "scene_color",
					.type = hl::ResourceType::Color,
					.format = "VK_FORMAT_B8G8R8A8_SRGB",
					.clear = VkClearValue{ .color = { { 0.02f, 0.03f, 0.06f, 1.0f } } }
				},
				hl::ResourceInfo{
					.name = "scene_depth",
					.type = hl::ResourceType::Depth,
					.format = "VK_FORMAT_D32_SFLOAT"
				}
			},
			.pipelineGroups =
			{
				{
					hl::PipelineInfo{
						.name = "entity_pipeline",
						.shaderVert = _engineConfig.RootPath + std::string("/data/shaders/galaxy.vert"),
						.shaderFrag = std::string(hl::RendererShaderRoot) + "/entity.frag",
						.descriptorSets =
						{
							hl::DescriptorSetInfo{
								.bindings =
								{
									hl::DescriptorBinding{
										.binding = 0,
										.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
										.stage = "VERTEX",
										.resource = cameraMatrixResourceId,
										.count = MAX_CAMERAS
									}
								}
							}
						},
						.vertexInputInfo = hl::VertexInputInfo{
							.attributes =
							{
								{
									.name = "inPosition",
									.format = hl::VertexAttributeFormat::Vec2,
									.location = 0,
									.offset = offsetof(hl::Vertex2, pos)
								}
							},
							.stride = sizeof(hl::Vertex2)
						},
						.depthState =
						{
							.writeEnable = true,
							.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
						},
						.rasterState =
						{
							.cullMode = VK_CULL_MODE_NONE
						},
						.enableBlending = false,
						.pushConstantSize = sizeof(GalaxyPushConstantObject)
					}
				}
			}
		};

		std::vector<hl::RenderpassInfo> renderpasses{
			sceneRenderpassInfo,
			hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId),
			hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "scene_color", "text_color" })
		};

		hl::ResourceContext resourceContext{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};
		loadTextResources(resourceManager, resourceContext);

		resourceManager.Load<hl::VertexArrayResource>(
			"unit_quad",
			resourceContext,
			std::vector<hl::Vertex2>{
				{ .pos = { -0.5f, -0.5f } },
				{ .pos = { 0.5f, -0.5f } },
				{ .pos = { 0.5f, 0.5f } },
				{ .pos = { -0.5f, -0.5f } },
				{ .pos = { 0.5f, 0.5f } },
				{ .pos = { -0.5f, 0.5f } }
			});

		const auto galaxyCameraIndex = static_cast<int>(getCameraIndex("Galaxy"));
		_galaxy.initialise(_engine.getTextSystem(), _world, galaxyCameraIndex);

		registerPipelineDraw(
			"entity_pipeline",
			[this](hl::PipelineDrawData& pdd)
			{
				_galaxy.drawGraph(pdd, *_resourceManager);
			});
		registerPipelineDraw(
			"sdf_text_pipeline",
			[this](hl::PipelineDrawData& pdd)
			{
				_galaxy.drawLabels(pdd, _engine.getTextSystem());
			});

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);
	}

	void GameEngineScene::update(uint32_t, float delta)
	{
		if (!hl::imgui::Context::wantCaptureMouse())
		{
			const auto mouse = _engine.getInputManager().getMousePosition();
			const bool middleDown = _engine.getInputManager().isButtonDown(GLFW_MOUSE_BUTTON_MIDDLE);
			if (middleDown)
			{
				if (_panning)
				{
					_galaxyCamera->panByScreenDelta(mouse - _lastMouse);
				}
				_panning = true;
				_lastMouse = mouse;
			}
			else
			{
				_panning = false;
			}

			if (!_panning && _engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1))
			{
				const glm::vec2 world = _galaxyCamera->worldFromScreen(mouse);
				const auto fleet = pickFleet(_world, world, GalaxyFleetSize * 0.5f, GalaxyFleetOffset);
				if (fleet.has_value())
				{
					_galaxy.setSelectedFleetId(fleet);
				}
				else
				{
					_galaxy.setSelectedSystemId(pickStarSystem(_world, world, GalaxyNodeSize * 0.5f));
				}
			}

			if (!_panning && _engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_2))
			{
				const auto fleetId = _galaxy.selectedFleetId();
				if (fleetId.has_value())
				{
					const glm::vec2 world = _galaxyCamera->worldFromScreen(mouse);
					const auto system = pickStarSystem(_world, world, GalaxyNodeSize * 0.5f);
					if (system.has_value())
					{
						_pendingCommands.push_back(GameCommand::moveFleetToSystem(
							*fleetId,
							*system,
							CommandSource::Player));
					}
				}
			}
		}
		else
		{
			_panning = false;
		}

		for (const auto& command : _pendingCommands)
		{
			execute(_world, command);
		}
		_pendingCommands.clear();

		const int ticks = _clock.takeTicks(delta);
		for (int i = 0; i < ticks; ++i)
		{
			stepGalaxy(_world);
		}
	}

	void GameEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (hl::imgui::Context::wantCaptureKeyboard())
			{
				return;
			}
			if (ke->GetKeyCode() == GLFW_KEY_BACKSPACE)
			{
				_engine.setScene(new TitleEngineScene(_engine, _engineConfig, _imgui));
			}
			else if (ke->GetKeyCode() == GLFW_KEY_SPACE)
			{
				_clock.togglePaused();
			}
		}
		else if (auto se = dynamic_cast<const hl::ScrollEvent*>(&event))
		{
			if (!hl::imgui::Context::wantCaptureMouse())
			{
				_galaxyCamera->zoomAtScreen(_engine.getInputManager().getMousePosition(), se->getY());
			}
		}
	}

}
