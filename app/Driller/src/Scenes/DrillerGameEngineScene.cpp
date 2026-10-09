#include <Scenes/DrillerGameEngineScene.hpp>
#include <Core/BuildJobClick.hpp>
#include <Core/DigJobClick.hpp>
#include <Core/GameCommand.hpp>
#include <Core/TileCoordinates.hpp>
#include <Services/UiService.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <GLFW/glfw3.h>
#include <helsinki/Engine/Input/InputManager.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <format>

namespace drl
{

	namespace
	{
		constexpr float kTileSize = 64.0f;
		constexpr float kTexSize = 1024.0f;
	}

	DrillerGameEngineScene::DrillerGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		Session& session
	) :
		EngineScene(engine),
		_engineConfig(engineConfig),
		_session(session),
		_terrainView(
			session.gameData().terrain,
			static_cast<float>(engineConfig.Width) * 0.5f,
			kTileSize,
			kTileSize),
		_buildingView(
			session.gameData().building,
			session.buildingPrototypeService(),
			static_cast<float>(engineConfig.Width) * 0.5f,
			kTileSize,
			kTileSize),
		_jobView(
			session.gameData().job,
			static_cast<float>(engineConfig.Width) * 0.5f,
			kTileSize,
			kTileSize),
		_workerView(
			session.gameData().worker,
			static_cast<float>(engineConfig.Width) * 0.5f,
			kTileSize,
			kTileSize),
		_shuttleView(
			session.gameData().shuttle,
			session.shuttlePrototypeService(),
			static_cast<float>(engineConfig.Width) * 0.5f,
			kTileSize,
			kTileSize)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
	}

	void DrillerGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		std::vector<hl::RenderpassInfo> renderpasses
		{
			hl::RenderpassInfo
			{
				.name = "sprite_pass",
				.inputs = {},
				.outputs =
				{
					hl::ResourceInfo
					{
						.name = "scene_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB",
						.clear = VkClearValue{.color = { 0.0f, 0.2f, 0.8f, 1.0f }}
					},
					hl::ResourceInfo
					{
						.name = "scene_depth",
						.type = hl::ResourceType::Depth,
						.format = "VK_FORMAT_D32_SFLOAT"
					}
				},
				.pipelineGroups =
				{
					{
						hl::PipelineInfo
						{
							.name = "sprite_pipeline",
							.shaderVert = _engineConfig.RootPath + "/data/shaders/sprites.vert",
							.shaderFrag = std::string(hl::RendererShaderRoot) + "/sprites.frag",
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
											.count = MAX_CAMERAS
										},
										hl::DescriptorBinding
										{
											.binding = 1,
											.type = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
											.stage = "VERTEX",
											.resource = "spritesheet_frame_ssbo"
										},
										hl::DescriptorBinding
										{
											.binding = 2,
											.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
											.stage = "FRAGMENT",
											.resource = "tile_sprite_sheet"
										}
									}
								}
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
							.enableBlending = true,
							.pushConstantSize = sizeof(hl::SpritePushConstantObject)
						}
					}
				}
			},
			hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId),
			hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "scene_color", "text_color" })
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
			"tile_sprite_sheet",
			resourceContext);
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);

		_spriteSheetSSBOResourceHandle = resourceManager.Load<hl::StorageBufferResource>(
			"spritesheet_frame_ssbo",
			resourceContext,
			sizeof(hl::FrameDataStorageBufferObject),
			256);

		{
			auto ssbo = _spriteSheetSSBOResourceHandle.Get();
			constexpr int kAtlasColumns = 16;
			constexpr int kAtlasRows = 16;
			for (int row = 0; row < kAtlasRows; ++row)
			{
				for (int col = 0; col < kAtlasColumns; ++col)
				{
					hl::FrameDataStorageBufferObject frame
					{
						.uvRect = glm::vec4(
							kTileSize * static_cast<float>(col),
							kTileSize * static_cast<float>(row),
							kTileSize * static_cast<float>(col + 1),
							kTileSize * static_cast<float>(row + 1)) / kTexSize
					};
					ssbo->writeToBuffer(&frame, static_cast<uint32_t>(col + row * kAtlasColumns));
				}
			}
		}

		resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
			"roboto",
			resourceContext);
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			"roboto",
			resourceContext);

		resourceManager.LoadLogical(
			hl::ResourceDefinition
			{
				.name = hl::TextSystem::RasterAtlasName,
				.type = "logical",
				.resources = { { .name = hl::MaterialSystem::FallbackTextureName, .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});
		resourceManager.LoadLogical(
			hl::ResourceDefinition
			{
				.name = hl::TextSystem::SdfAtlasName,
				.type = "logical",
				.resources = { { .name = "roboto", .type = "texture" } }
			},
			[&](const hl::ResourceDefinition::Child& child)
			{
				return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});

		{
			auto entity = _scene.addEntity("hud_ore");
			entity->AddTag("TEXT");
			entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(4.0f, 0.0f, 0.0f));
			entity->AddComponent<hl::TextComponent>()->setString(
				_engine.getTextSystem(),
				"Ore: 0",
				"roboto",
				32);
			entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
		{
			auto entity = _scene.addEntity("hud_money");
			entity->AddTag("TEXT");
			entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(4.0f, 32.0f, 0.0f));
			entity->AddComponent<hl::TextComponent>()->setString(
				_engine.getTextSystem(),
				"Money: 500",
				"roboto",
				32);
			entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);

		registerPipelineDraw(
			"sprite_pipeline",
			[this](hl::PipelineDrawData& pdd) -> void
			{
				_terrainView.draw(pdd);
				_buildingView.draw(pdd);
				_jobView.draw(pdd);
				_workerView.draw(pdd);
				_shuttleView.draw(pdd);
			});
	}

	void DrillerGameEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		_session.game().update(delta);

		struct SceneUiInput : IUiInput
		{
			explicit SceneUiInput(const hl::InputManager& inputManager)
				: inputManager(inputManager)
			{
			}

			bool isKeyDown(int key) const override
			{
				return inputManager.isKeyDown(key);
			}

			const hl::InputManager& inputManager;
		};

		SceneUiInput uiInput(_engine.getInputManager());
		_session.uiService().update(uiInput);

		auto& economy = _session.economyService();
		_scene.getEntity("hud_ore")->GetComponent<hl::TextComponent>()->setString(
			_engine.getTextSystem(),
			std::format("Ore: {}", economy.get(ResourceOre)),
			"roboto",
			32);
		_scene.getEntity("hud_money")->GetComponent<hl::TextComponent>()->setString(
			_engine.getTextSystem(),
			std::format("Money: {}", economy.get(ResourceMoney)),
			"roboto",
			32);

		if (!_engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1))
		{
			return;
		}

		const auto tile = pixelToTile(
			_engine.getInputManager().getMousePosition(),
			_terrainView.originX(),
			_terrainView.originY(),
			_terrainView.tileSize());

		if (_session.uiService().getCurrentState() == UiState::PlacingBuilding)
		{
			if (tile.y >= 0 && tile.x != 0)
			{
				enqueueBuildJob(
					_session.commandService(),
					_session.terrainService(),
					tile.y,
					tile.x,
					_session.uiService().getActiveBuildingType());
				const bool shiftRange =
					_engine.getInputManager().isKeyDown(GLFW_KEY_LEFT_SHIFT)
					|| _engine.getInputManager().isKeyDown(GLFW_KEY_RIGHT_SHIFT);
				if (!shiftRange)
				{
					_session.uiService().clearActiveBuilding();
				}
			}
			return;
		}

		if (tile.y < 0)
		{
			return;
		}

		if (tile.x == 0)
		{
			_session.commandService().execute(GameCommand::digShaft(
				tile.y,
				CommandSource::Player,
				CommandContext::DiggingShaft));
		}
		else
		{
			const bool shiftRange =
				_engine.getInputManager().isKeyDown(GLFW_KEY_LEFT_SHIFT)
				|| _engine.getInputManager().isKeyDown(GLFW_KEY_RIGHT_SHIFT);
			enqueueDigJobs(
				_session.commandService(),
				_session.terrainService(),
				tile.y,
				tile.x,
				shiftRange);
		}
	}

}
