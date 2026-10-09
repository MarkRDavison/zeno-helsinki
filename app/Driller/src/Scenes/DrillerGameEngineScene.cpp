#include <Scenes/DrillerGameEngineScene.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>

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
		_session(session)
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
							.rasterState =
							{
								.cullMode = VK_CULL_MODE_NONE
							},
							.enableBlending = false,
							.pushConstantSize = sizeof(hl::SpritePushConstantObject)
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
			"tile_sprite_sheet",
			resourceContext);
		resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
			hl::MaterialSystem::FallbackTextureName,
			resourceContext);

		_spriteSheetSSBOResourceHandle = resourceManager.Load<hl::StorageBufferResource>(
			"spritesheet_frame_ssbo",
			resourceContext,
			sizeof(hl::FrameDataStorageBufferObject),
			128);

		{
			auto ssbo = _spriteSheetSSBOResourceHandle.Get();
			const auto cell = [&](int col, int row)
			{
				return hl::FrameDataStorageBufferObject
				{
					.uvRect = glm::vec4(
						kTileSize * static_cast<float>(col),
						kTileSize * static_cast<float>(row),
						kTileSize * static_cast<float>(col + 1),
						kTileSize * static_cast<float>(row + 1)) / kTexSize
				};
			};
			std::vector<hl::FrameDataStorageBufferObject> frameData
			{
				cell(0, 0),
				cell(1, 0),
				cell(2, 0),
				cell(0, 1),
			};
			for (uint32_t i = 0; i < static_cast<uint32_t>(frameData.size()); ++i)
			{
				ssbo->writeToBuffer(&frameData[i], i);
			}
		}

		const auto addTileSprite = [&](const std::string& name, float tileX, float tileY, int frame)
		{
			auto entity = _scene.addEntity(name);
			entity->AddTag("SPRITE");
			entity->AddComponent<hl::TransformComponent>()->SetPosition(
				glm::vec3(tileX * kTileSize, tileY * kTileSize, 0.0f));
			entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(frame);
		};
		addTileSprite("sample0", 1.0f, 0.0f, 0);
		addTileSprite("sample1", 2.0f, 0.0f, 1);
		addTileSprite("sample2", 1.0f, 1.0f, 2);
		addTileSprite("sample3", 3.0f, 1.0f, 3);

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
			[](hl::PipelineDrawData& pdd) -> void
			{
				for (const auto& entity : pdd.scene->getEntities())
				{
					if (!entity->HasComponents<hl::TransformComponent, hl::SpriteComponent>())
					{
						continue;
					}

					const auto& transform = entity->GetComponent<hl::TransformComponent>();
					const auto& sprite = entity->GetComponent<hl::SpriteComponent>();

					auto pc = hl::SpritePushConstantObject
					{
						.model = transform->GetTransformMatrix(),
						.size = glm::vec2(kTileSize, kTileSize),
						.frameIndex = sprite->getFrameDataIndex()
					};

					vkCmdPushConstants(
						pdd.commandBuffer,
						pdd.pipeline->getPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(hl::SpritePushConstantObject),
						&pc);

					auto descriptorSet = pdd.pipeline->getDescriptorSet(pdd.currentFrame);
					vkCmdBindDescriptorSets(
						pdd.commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						pdd.pipeline->getPipelineLayout(),
						0,
						1,
						&descriptorSet,
						0,
						nullptr);

					vkCmdDraw(pdd.commandBuffer, 6, 1, 0, 0);
				}
			});
	}

	void DrillerGameEngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
	}

}
