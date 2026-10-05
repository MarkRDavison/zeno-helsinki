#include "Scenes/TowerDefenseGameEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <GroundPick.hpp>
#include <SceneCatalog.hpp>
#include <SunUniformBufferObject.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/TileComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <Systems/PathFollowSystem.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>

namespace tower
{

	TowerDefenseGameEngineScene::TowerDefenseGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig)
	{
		_cameras.insert({ "Ui", new hl::Camera2D() });
		_cameras.insert({ "Default", new hl::Camera(
			glm::vec3(0.0f, 16.0f, 16.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			-90.0f,
			-45.0f) });
	}

	TowerDefenseGameEngineScene::~TowerDefenseGameEngineScene()
	{
		_sceneHost.onSceneDestroyed();
	}

	std::vector<hl::RenderpassInfo> TowerDefenseGameEngineScene::buildRenderpasses() const
	{
		return
		{
			hl::RenderpassInfo
			{
				.name = "scene_pass",
				.inputs = {},
				.outputs =
				{
					hl::ResourceInfo
					{
						.name = "scene_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB",
						.useMultiSampling = true
					},
					hl::ResourceInfo
					{
						.name = "scene_depth",
						.type = hl::ResourceType::Depth,
						.format = "VK_FORMAT_D32_SFLOAT",
						.useMultiSampling = true
					}
				},
				.pipelineGroups =
				{
					{
						hl::PipelineInfo
						{
							.name = "model_pipeline",
							.shaderVert = _engineConfig.RootPath + std::string("/data/shaders/material_pbr.vert"),
							.shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/material_pbr.frag"),
							.descriptorSets =
							{
								hl::DescriptorSetInfo
								{
									.name = "model_uniforms",
									.bindings =
									{
										hl::DescriptorBinding
										{
											.binding = 0,
											.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
											.stage = "VERTEX&FRAGMENT",
											.resource = "camera_matrix_ubo",
											.count = MAX_CAMERAS
										},
										hl::DescriptorBinding
										{
											.binding = 1,
											.type = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
											.stage = "VERTEX&FRAGMENT",
											.resource = "material_ssbo"
										},
										hl::DescriptorBinding
										{
											.binding = 2,
											.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
											.stage = "FRAGMENT",
											.resource = "white"
										},
										hl::DescriptorBinding
										{
											.binding = 3,
											.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
											.stage = "FRAGMENT",
											.resource = "sun_ubo",
											.count = 1,
											.updateFrequency = hl::DescriptorUpdateFrequency::PerFrame
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
										.format = hl::VertexAttributeFormat::Vec3,
										.location = 0,
										.offset = offsetof(hl::Vertex, pos)
									},
									{
										.name = "inColor",
										.format = hl::VertexAttributeFormat::Vec3,
										.location = 1,
										.offset = offsetof(hl::Vertex, color)
									},
									{
										.name = "inTexCoord",
										.format = hl::VertexAttributeFormat::Vec2,
										.location = 2,
										.offset = offsetof(hl::Vertex, texCoord)
									},
									{
										.name = "inNormal",
										.format = hl::VertexAttributeFormat::Vec3,
										.location = 3,
										.offset = offsetof(hl::Vertex, normal)
									}
								},
								.stride = sizeof(hl::Vertex)
							},
							.rasterState =
							{
								.cullMode = VK_CULL_MODE_NONE
							},
							.enableBlending = false,
							.pushConstantSize = sizeof(hl::MaterialPushConstantObject)
						}
					}
				}
			},
			hl::RenderpassInfo
			{
				.name = "postprocess_pass",
				.inputs = { "scene_color" },
				.outputs =
				{
					hl::ResourceInfo
					{
						.name = "post_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB"
					}
				},
				.pipelineGroups =
				{
					{
						hl::PipelineInfo
						{
							.name = "postprocess_pipeline",
							.shaderVert = _engineConfig.RootPath + std::string("/data/shaders/post_process.vert"),
							.shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/post_process.frag"),
							.descriptorSets =
							{
								hl::DescriptorSetInfo
								{
									.name = "input_sampler",
									.bindings =
									{
										hl::DescriptorBinding
										{
											.binding = 0,
											.type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
											.stage = "FRAGMENT",
											.resource = "scene_color"
										}
									}
								}
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
							.enableBlending = false
						}
					}
				}
			},
			hl::RenderpassInfo
			{
				.name = "ui_pass",
				.inputs = {},
				.outputs =
				{
					hl::ResourceInfo
					{
						.name = "ui_color",
						.type = hl::ResourceType::Color,
						.format = "VK_FORMAT_B8G8R8A8_SRGB",
						.clear = VkClearValue{.color = {{ 0.0f, 0.0f, 0.0f, 0.0f }}}
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
											.resource = "camera_matrix_ubo",
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
											.updateFrequency = hl::DescriptorUpdateFrequency::Static
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
							.enableBlending = true
						}
					}
				}
			},
			hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "post_color", "ui_color" })
		};
	}

	void TowerDefenseGameEngineScene::spawnScene(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		_sunUbo = resourceManager.Load<hl::UniformBufferResource>(
			"sun_ubo",
			resourceContext,
			sizeof(SunUniformBufferObject),
			MAX_FRAMES_IN_FLIGHT,
			1);

		loadMenuUiSheet(
			resourceManager,
			resourceContext,
			"ui_sheet",
			{
				hl::ResourceDefinition::Child{ .name = "white", .type = "texture" },
				hl::ResourceDefinition::Child{ .name = "roboto", .type = "texture" }
			});

		spawnBoard(resourceManager, resourceContext);
		resourceManager.Load<hl::ModelResource>(TurretModelId, resourceContext);
		spawnTower(resourceManager, TurretTile.x, TurretTile.z);
		spawnCreep(resourceManager, resourceContext);
		spawnMarker(resourceManager, resourceContext);

		_scene.addSystem(new PathFollowSystem(_scene));
	}

	void TowerDefenseGameEngineScene::spawnBoard(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		auto black = resourceManager.Load<hl::ModelResource>(TileBlackModelId, resourceContext);
		auto white = resourceManager.Load<hl::ModelResource>(TileWhiteModelId, resourceContext);

		for (int tz = 0; tz < BoardSize; ++tz)
		{
			for (int tx = 0; tx < BoardSize; ++tx)
			{
				const bool dark = ((tx + tz) & 1) != 0;
				auto* entity = _scene.addEntity();
				entity->AddTag(TileTag);
				if (isPathTile(tx, tz))
				{
					entity->AddTag(PathTag);
				}
				else
				{
					entity->AddTag(BuildableTag);
				}

				auto* tile = entity->AddComponent<TileComponent>();
				tile->x = tx;
				tile->z = tz;
				tile->path = isPathTile(tx, tz);

				entity->AddComponent<hl::TransformComponent>()->SetPosition(tileCenter(tx, tz));
				entity->AddComponent<hl::ModelComponent>()->setModelId(
					dark ? black->GetId() : white->GetId());
			}
		}
	}

	void TowerDefenseGameEngineScene::spawnTower(hl::ResourceManager& resourceManager, int tx, int tz)
	{
		auto* model = resourceManager.GetResource<hl::ModelResource>(TurretModelId);
		auto* entity = _scene.addEntity();
		entity->AddTag(TowerTag);
		auto* tower = entity->AddComponent<TowerComponent>();
		tower->x = tx;
		tower->z = tz;
		entity->AddComponent<hl::TransformComponent>()->SetPosition(tileCenter(tx, tz));
		entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
	}

	bool TowerDefenseGameEngineScene::isOccupied(int tx, int tz) const
	{
		for (auto* entity : _scene.getEntitiesByTag(TowerTag))
		{
			const auto* tower = entity->GetComponent<TowerComponent>();
			if (tower != nullptr && tower->x == tx && tower->z == tz)
			{
				return true;
			}
		}

		return false;
	}

	void TowerDefenseGameEngineScene::spawnCreep(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		auto modelHandle = resourceManager.Load<hl::ModelResource>(CreepModelId, resourceContext);
		const auto start = PathWaypoints[0];
		auto* entity = _scene.addEntity("creep");
		entity->AddTag(CreepTag);
		auto* transform = entity->AddComponent<hl::TransformComponent>();
		transform->SetPosition(tileCenter(start.x, start.z));
		transform->SetScale(CreepScale);
		entity->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
		auto* follow = entity->AddComponent<PathFollowComponent>();
		follow->fromIndex = 0;
		follow->t = 0.0f;
		follow->speed = CreepSpeed;
	}

	void TowerDefenseGameEngineScene::spawnMarker(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		auto modelHandle = resourceManager.Load<hl::ModelResource>(
			MarkerModelId,
			resourceContext);

		_marker = _scene.addEntity("marker");
		_marker->AddTag(MarkerTag);
		auto* transform = _marker->AddComponent<hl::TransformComponent>();
		transform->SetPosition(tileCenter(MarkerStartTile.x, MarkerStartTile.z, MarkerY));
		transform->SetScale(MarkerScale);
		_marker->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
	}

	void TowerDefenseGameEngineScene::tryHandleBoardClick()
	{
		const auto& input = _engine.getInputManager();
		const bool released = input.isButtonReleased(GLFW_MOUSE_BUTTON_1);
		if (!released || _marker == nullptr)
		{
			return;
		}

		auto it = _cameras.find("Default");
		if (it == _cameras.end())
		{
			return;
		}

		const auto* camera = static_cast<hl::Camera*>(it->second);
		const Ray ray = rayFromCameraMouse(
			*camera,
			input.getMousePosition(),
			input.getWindowSize());

		const auto hit = intersectGroundY0(ray);
		if (!hit)
		{
			return;
		}

		const auto tile = worldToTile(*hit);
		if (!tile)
		{
			return;
		}

		_marker->GetComponent<hl::TransformComponent>()->SetPosition(
			tileCenter(tile->x, tile->z, MarkerY));

		if (isPathTile(tile->x, tile->z) || isOccupied(tile->x, tile->z) || _gold < TowerCost)
		{
			return;
		}

		_gold -= TowerCost;
		spawnTower(*_resourceManager, tile->x, tile->z);
	}

	void TowerDefenseGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		auto renderpasses = buildRenderpasses();

		hl::ResourceContext resourceContext
		{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};

		spawnScene(resourceManager, resourceContext);

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);

		buildHud(resourceManager.GetResource<hl::FontResource>("roboto"));
		_uiBatch.initialise(device);

		registerPipelineDraw("model_pipeline", [this](hl::PipelineDrawData& pdd)
			{
				const auto cameraIndex = static_cast<uint32_t>(getCameraIndex("Default"));
				for (const auto& entity : pdd.scene->getEntities())
				{
					if (!entity->HasComponents<hl::TransformComponent, hl::ModelComponent>())
					{
						continue;
					}

					const auto* transform = entity->GetComponent<hl::TransformComponent>();
					const auto* model = entity->GetComponent<hl::ModelComponent>();
					const auto* modelResource = _resourceManager->GetResource<hl::ModelResource>(model->getModelId());
					auto pc = hl::MaterialPushConstantObject
					{
						.model = transform->GetTransformMatrix()
					};
					pc.pad[0] = cameraIndex;

					for (const auto& mesh : modelResource->getMeshes())
					{
						pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(mesh.materialName);
						vkCmdPushConstants(
							pdd.commandBuffer,
							pdd.pipeline->getPipelineLayout(),
							VK_SHADER_STAGE_VERTEX_BIT,
							0,
							sizeof(hl::MaterialPushConstantObject),
							&pc);

						VkBuffer vertexBuffers[] = { mesh._vertexBuffer._buffer };
						VkDeviceSize offsets[] = { 0 };
						vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);
						vkCmdBindIndexBuffer(
							pdd.commandBuffer,
							mesh._indexBuffer._buffer,
							0,
							VK_INDEX_TYPE_UINT32);

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

						vkCmdDrawIndexed(pdd.commandBuffer, mesh._indexCount, 1, 0, 0, 0);
					}
				}
			});

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd)
			{
				_uiBatch.draw(pdd);
			});
	}

	void TowerDefenseGameEngineScene::buildHud(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();
		_goldLabel = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
		_goldLabel->color = { 1.0f, 1.0f, 1.0f };
		_goldLabel->setText("Gold: " + std::to_string(_gold), 24);
	}

	void TowerDefenseGameEngineScene::rebuildHud()
	{
		_goldLabel->setText("Gold: " + std::to_string(_gold), 24);
		hl::ui::prepareTree(*_layoutRoot);

		const glm::vec2 size = _goldLabel->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		_goldLabel->node().setTopLeft(size);
		_goldLabel->node().relative = { 16.0f, 16.0f };
		_goldLabel->node().intrinsicSize.reset();

		const auto fb = _engine.getInputManager().getFramebufferSize();
		hl::ui::layout(*_layoutRoot, hl::ui::Box{ 0.0f, 0.0f, fb.x, fb.y });

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);
	}

	void TowerDefenseGameEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		tryHandleBoardClick();
		_scene.update(delta);
		rebuildHud();
	}

	void TowerDefenseGameEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		if (_sunUbo)
		{
			SunUniformBufferObject ubo{};
			const auto dir = glm::normalize(glm::vec3(0.45f, 0.85f, 0.30f));
			ubo.direction = glm::vec4(dir, 1.0f);
			ubo.color = glm::vec4(1.0f, 0.97f, 0.90f, 0.35f);
			ubo.ambient = glm::vec4(0.18f, 0.18f, 0.18f, 0.0f);
			_sunUbo.Get()->getUniformBuffer(currentFrame).writeToBuffer(&ubo, 0);
		}

		_uiBatch.updateGpuResources(currentFrame);
	}

	void TowerDefenseGameEngineScene::additionalCleanup()
	{
		_uiBatch.destroy();
	}
}
