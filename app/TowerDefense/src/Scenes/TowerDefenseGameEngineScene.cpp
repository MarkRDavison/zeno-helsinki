#include "Scenes/TowerDefenseGameEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <AudioCatalog.hpp>
#include <GroundPick.hpp>
#include <SceneCatalog.hpp>
#include <SunUniformBufferObject.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/TileComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <Systems/PathFollowSystem.hpp>
#include <Systems/TowerFireSystem.hpp>
#include <Systems/ProjectileSystem.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/Material.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/System/Events/ScrollEvent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Ui/Widget.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <string>

namespace tower
{

	TowerDefenseGameEngineScene::TowerDefenseGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		GameStateService& gameState,
		WaveService& wave,
		hl::audio::Audio& audio
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_gameState(gameState),
		_wave(wave),
		_audio(audio)
	{
		_cameras.insert({ "Ui", new hl::Camera2D() });
		_cameras.insert({ "Default", new hl::Camera(
			glm::vec3(0.0f, 16.0f, 16.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			-90.0f,
			-45.0f) });
		_cameraDistance = glm::length(glm::vec3(0.0f, 16.0f, 16.0f));
		_cameraDistanceTarget = _cameraDistance;
		_engine.getEventBus().AddListener(this);
	}

	TowerDefenseGameEngineScene::~TowerDefenseGameEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	std::vector<hl::RenderpassInfo> TowerDefenseGameEngineScene::buildRenderpasses() const
	{
		const auto root = _engineConfig.RootPath + std::string("/data/shaders/");
		const hl::VertexInputInfo pbrVertexInput
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
		};
		const hl::DescriptorSetInfo pbrDescriptors
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
		};

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
							.shaderVert = root + "material_pbr.vert",
							.shaderFrag = root + "material_pbr.frag",
							.descriptorSets = { pbrDescriptors },
							.vertexInputInfo = pbrVertexInput,
							.rasterState =
							{
								.cullMode = VK_CULL_MODE_NONE
							},
							.enableBlending = false,
							.pushConstantSize = sizeof(hl::MaterialPushConstantObject)
						},
						hl::PipelineInfo
						{
							.name = "ghost_pipeline",
							.shaderVert = root + "material_pbr.vert",
							.shaderFrag = root + "material_pbr_ghost.frag",
							.descriptorSets = { pbrDescriptors },
							.vertexInputInfo = pbrVertexInput,
							.depthState =
							{
								.testEnable = true,
								.writeEnable = false
							},
							.rasterState =
							{
								.cullMode = VK_CULL_MODE_NONE
							},
							.enableBlending = true,
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
		resourceManager.Load<hl::ModelResource>(CreepModelId, resourceContext);
		_engine.getMaterialSystem().addMaterial(hl::Material{
			.name = CreepRunner.material,
			.diffuse = CreepRunner.kd
		});
		_engine.getMaterialSystem().addMaterial(hl::Material{
			.name = CreepTank.material,
			.diffuse = CreepTank.kd
		});
		spawnMarker(resourceManager, resourceContext);
		spawnGhost(resourceManager);
		spawnRangeRing(resourceManager, resourceContext);
		spawnPathRibbons(resourceManager, resourceContext);

		auto* pathFollow = new PathFollowSystem(_scene);
		pathFollow->onLeak = [this]() { onCreepLeaked(); };
		_scene.addSystem(pathFollow);
		_scene.addSystem(new TowerFireSystem(_scene, resourceManager));
		auto* projectiles = new ProjectileSystem(_scene);
		projectiles->onKill = [this]() { onCreepKilled(); };
		_scene.addSystem(projectiles);
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
		_audio.play(CuePlace);
	}

	bool TowerDefenseGameEngineScene::isOccupied(int tx, int tz) const
	{
		return towerAt(tx, tz) != nullptr;
	}

	hl::Entity* TowerDefenseGameEngineScene::towerAt(int tx, int tz) const
	{
		for (auto* entity : _scene.getEntitiesByTag(TowerTag))
		{
			const auto* tower = entity->GetComponent<TowerComponent>();
			if (tower != nullptr && tower->x == tx && tower->z == tz)
			{
				return entity;
			}
		}

		return nullptr;
	}

	void TowerDefenseGameEngineScene::flashInvalid(int tx, int tz)
	{
		_invalidFlashRemaining = InvalidFlashSeconds;
		_flashTower = towerAt(tx, tz);
		_flashGhost = _flashTower == nullptr;
	}

	bool TowerDefenseGameEngineScene::invalidFlashBlinkOn() const
	{
		return _invalidFlashRemaining > 0.0f
			&& std::fmod(_invalidFlashRemaining, 0.1f) > 0.05f;
	}

	void TowerDefenseGameEngineScene::spawnCreep()
	{
		auto* model = _resourceManager->GetResource<hl::ModelResource>(CreepModelId);
		const auto& def = _wave.nextCreep();
		const auto start = PathWaypoints[0];
		auto* entity = _scene.addEntity();
		entity->AddTag(CreepTag);
		auto* transform = entity->AddComponent<hl::TransformComponent>();
		transform->SetPosition(tileCenter(start.x, start.z));
		transform->SetScale(CreepScale);
		entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		entity->AddComponent<CreepComponent>()->material = def.material;
		auto* follow = entity->AddComponent<PathFollowComponent>();
		follow->fromIndex = 0;
		follow->t = 0.0f;
		follow->speed = def.speed;
		auto* health = entity->AddComponent<HealthComponent>();
		health->max = def.health;
		health->current = def.health;
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

	void TowerDefenseGameEngineScene::spawnGhost(hl::ResourceManager& resourceManager)
	{
		auto* materials = &_engine.getMaterialSystem();
		materials->addMaterial(hl::Material{
			.name = GhostOkMaterial,
			.diffuse = { 0.35f, 0.85f, 0.40f }
		});
		materials->addMaterial(hl::Material{
			.name = GhostBadMaterial,
			.diffuse = { 0.85f, 0.25f, 0.25f }
		});

		auto* model = resourceManager.GetResource<hl::ModelResource>(TurretModelId);
		_ghost = _scene.addEntity();
		_ghost->AddTag(GhostTag);
		_ghost->AddComponent<hl::TransformComponent>()->SetPosition(tileCenter(0, 0));
		_ghost->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		_ghostVisible = false;
	}

	void TowerDefenseGameEngineScene::spawnRangeRing(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		auto modelHandle = resourceManager.Load<hl::ModelResource>(
			RangeRingModelId,
			resourceContext);

		_rangeRing = _scene.addEntity();
		_rangeRing->AddTag(RangeRingTag);
		auto* transform = _rangeRing->AddComponent<hl::TransformComponent>();
		transform->SetPosition(tileCenter(0, 0, RangeRingY));
		transform->SetScale(glm::vec3(TowerRange, 1.0f, TowerRange));
		_rangeRing->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
	}

	void TowerDefenseGameEngineScene::spawnPathRibbons(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		_engine.getMaterialSystem().addMaterial(hl::Material{
			.name = PathRibbonMaterial,
			.diffuse = { 0.95f, 0.65f, 0.15f }
		});

		auto modelHandle = resourceManager.Load<hl::ModelResource>(
			PathRibbonModelId,
			resourceContext);

		auto spawnLeg = [&](const glm::vec3& mid, float yawDegrees, float length)
		{
			auto* entity = _scene.addEntity();
			entity->AddTag(PathRibbonTag);
			auto* transform = entity->AddComponent<hl::TransformComponent>();
			transform->SetPosition(mid);
			transform->SetRotation(glm::vec3(0.0f, yawDegrees, 0.0f));
			transform->SetScale(glm::vec3(PathRibbonWidth, 1.0f, length));
			entity->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
		};

		const glm::vec3 verticalA = tileCenter(0, 0, PathRibbonY);
		glm::vec3 verticalB = tileCenter(0, BoardSize - 1, PathRibbonY);
		verticalB.z += PathRibbonWidth * 0.5f;
		spawnLeg(
			(verticalA + verticalB) * 0.5f,
			0.0f,
			PathRibbonLength + PathRibbonWidth * 0.5f);

		const glm::vec3 corner = tileCenter(0, BoardSize - 1, PathRibbonY);
		const glm::vec3 horizontalB = tileCenter(BoardSize - 1, BoardSize - 1, PathRibbonY);
		glm::vec3 horizontalA = corner;
		horizontalA.x += PathRibbonWidth * 0.5f;
		spawnLeg(
			(horizontalA + horizontalB) * 0.5f,
			90.0f,
			PathRibbonLength - PathRibbonWidth * 0.5f);
	}

	std::optional<TileCoord> TowerDefenseGameEngineScene::hoveredTile() const
	{
		auto it = _cameras.find("Default");
		if (it == _cameras.end())
		{
			return std::nullopt;
		}

		const auto& input = _engine.getInputManager();
		const auto* camera = static_cast<hl::Camera*>(it->second);
		const Ray ray = rayFromCameraMouse(
			*camera,
			input.getMousePosition(),
			input.getWindowSize());
		const auto hit = intersectGroundY0(ray);
		if (!hit)
		{
			return std::nullopt;
		}

		return worldToTile(*hit);
	}

	void TowerDefenseGameEngineScene::updateGhost()
	{
		_ghostVisible = false;
		_ghostPlaceable = false;
		if (_ghost == nullptr || matchEnded() || _wave.inCombat() || uiBlocksBoardClick())
		{
			return;
		}

		const auto tile = hoveredTile();
		if (!tile || isOccupied(tile->x, tile->z))
		{
			return;
		}

		_ghost->GetComponent<hl::TransformComponent>()->SetPosition(
			tileCenter(tile->x, tile->z));
		if (_rangeRing != nullptr)
		{
			auto* ring = _rangeRing->GetComponent<hl::TransformComponent>();
			ring->SetPosition(tileCenter(tile->x, tile->z, RangeRingY));
			ring->SetScale(glm::vec3(TowerRange, 1.0f, TowerRange));
		}
		_ghostPlaceable = !isPathTile(tile->x, tile->z) && _gameState.gold() >= TowerCost;
		_ghostVisible = true;
	}

	void TowerDefenseGameEngineScene::tryHandleBoardClick()
	{
		if (matchEnded() || uiBlocksBoardClick())
		{
			return;
		}
		const auto& input = _engine.getInputManager();
		const bool released = input.isButtonReleased(GLFW_MOUSE_BUTTON_1);
		if (!released || _marker == nullptr)
		{
			return;
		}

		const auto tile = hoveredTile();
		if (!tile)
		{
			return;
		}

		_marker->GetComponent<hl::TransformComponent>()->SetPosition(
			tileCenter(tile->x, tile->z, MarkerY));

		if (_wave.inCombat())
		{
			return;
		}

		if (auto* tower = towerAt(tile->x, tile->z))
		{
			_gameState.addGold(TowerSellRefund);
			if (_flashTower == tower)
			{
				_flashTower = nullptr;
				_invalidFlashRemaining = 0.0f;
				_flashGhost = false;
			}
			_scene.removeEntity(tower->Id);
			return;
		}

		if (isPathTile(tile->x, tile->z) || !_gameState.trySpend(TowerCost))
		{
			flashInvalid(tile->x, tile->z);
			return;
		}

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
				const bool showPath = _engine.getInputManager().isKeyDown(GLFW_KEY_P);
				for (const auto& entity : pdd.scene->getEntities())
				{
					if (entity->HasTag(GhostTag)
						|| entity->HasTag(RangeRingTag)
						|| !entity->HasComponents<hl::TransformComponent, hl::ModelComponent>())
					{
						continue;
					}

					if (entity.get() == _flashTower && _invalidFlashRemaining > 0.0f)
					{
						continue;
					}

					if (entity->HasTag(PathRibbonTag) && !showPath)
					{
						continue;
					}

					const auto* transform = entity->GetComponent<hl::TransformComponent>();
					const auto* model = entity->GetComponent<hl::ModelComponent>();
					const auto* modelResource = _resourceManager->GetResource<hl::ModelResource>(model->getModelId());
					if (modelResource == nullptr)
					{
						continue;
					}

					auto pc = hl::MaterialPushConstantObject
					{
						.model = transform->GetTransformMatrix()
					};
					pc.pad[0] = cameraIndex;

					for (const auto& mesh : modelResource->getMeshes())
					{
						const auto* creep = entity->GetComponent<CreepComponent>();
						pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(
							creep != nullptr && creep->material != nullptr
								? creep->material
								: mesh.materialName);
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

		registerPipelineDraw("ghost_pipeline", [this](hl::PipelineDrawData& pdd)
			{
				const auto cameraIndex = static_cast<uint32_t>(getCameraIndex("Default"));
				const uint32_t badMaterial = _engine.getMaterialSystem().getMaterialIndex(GhostBadMaterial);
				const bool blinkOn = invalidFlashBlinkOn();
				const bool flashGhost = _flashGhost && _invalidFlashRemaining > 0.0f;

				auto drawGhostLit = [&](hl::Entity* entity, uint32_t materialIndex)
				{
					if (entity == nullptr
						|| !entity->HasComponents<hl::TransformComponent, hl::ModelComponent>())
					{
						return;
					}

					const auto* transform = entity->GetComponent<hl::TransformComponent>();
					const auto* model = entity->GetComponent<hl::ModelComponent>();
					const auto* modelResource = _resourceManager->GetResource<hl::ModelResource>(model->getModelId());
					if (modelResource == nullptr)
					{
						return;
					}

					auto pc = hl::MaterialPushConstantObject
					{
						.model = transform->GetTransformMatrix()
					};
					pc.pad[0] = cameraIndex;
					pc.materialIndex = materialIndex;

					for (const auto& mesh : modelResource->getMeshes())
					{
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
				};

				if (flashGhost)
				{
					if (blinkOn)
					{
						drawGhostLit(_ghost, badMaterial);
						drawGhostLit(_rangeRing, badMaterial);
					}
				}
				else if (_ghostVisible)
				{
					const char* materialName = _ghostPlaceable ? GhostOkMaterial : GhostBadMaterial;
					const uint32_t ghostMaterial = _engine.getMaterialSystem().getMaterialIndex(materialName);
					drawGhostLit(_ghost, ghostMaterial);
					drawGhostLit(_rangeRing, ghostMaterial);
				}

				if (_flashTower != nullptr && _invalidFlashRemaining > 0.0f && blinkOn)
				{
					drawGhostLit(_flashTower, badMaterial);
				}
			});

		registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd)
			{
				_uiBatch.draw(pdd);
			});
	}

	bool TowerDefenseGameEngineScene::matchEnded() const
	{
		return _gameState.matchEnded();
	}

	bool TowerDefenseGameEngineScene::uiBlocksBoardClick() const
	{
		const auto pointer = readMenuPointer(_engine);
		if (hl::ui::hitTest(*_layoutRoot, pointer.position) != nullptr)
		{
			return true;
		}

		if (matchEnded() && hl::ui::hitTest(*_overlayRoot, pointer.position) != nullptr)
		{
			return true;
		}

		return false;
	}

	void TowerDefenseGameEngineScene::startWave()
	{
		if (!_wave.tryStart())
		{
			return;
		}

		tickWave(0.0f);
	}

	void TowerDefenseGameEngineScene::tickWave(float delta)
	{
		_wave.tick(delta);
		while (_wave.takeSpawn())
		{
			spawnCreep();
		}
	}

	bool TowerDefenseGameEngineScene::boardHasLiveCreep() const
	{
		for (auto* creep : _scene.getEntitiesByTag(CreepTag))
		{
			if (!_scene.isPendingRemoval(creep->Id))
			{
				return true;
			}
		}

		return false;
	}

	void TowerDefenseGameEngineScene::tryClearWave()
	{
		if (!_wave.tryClear(!boardHasLiveCreep()))
		{
			return;
		}

		if (!_gameState.won())
		{
			return;
		}

		_overlayHeading->setText("You Win", 64);
		_audio.play(CueWin);
	}

	void TowerDefenseGameEngineScene::onCreepLeaked()
	{
		if (matchEnded())
		{
			return;
		}

		_gameState.onLeak();
		_audio.play(CueLeak);
		if (!_gameState.matchEnded())
		{
			return;
		}

		_overlayHeading->setText("Game Over", 64);
	}

	void TowerDefenseGameEngineScene::onCreepKilled()
	{
		_gameState.addGold(KillGold);
	}

	void TowerDefenseGameEngineScene::buildHud(hl::FontResource* font)
	{
		_typeface = std::make_unique<FontTypeface>(font);
		_layoutRoot = std::make_unique<hl::ui::Node>();
		_layoutRoot->setFillParent();
		_goldLabel = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
		_goldLabel->color = { 1.0f, 1.0f, 1.0f };
		_goldLabel->setText("Gold: " + std::to_string(_gameState.gold()), 24);

		_livesLabel = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
		_livesLabel->color = { 1.0f, 1.0f, 1.0f };
		_livesLabel->setText("Lives: " + std::to_string(_gameState.lives()), 24);

		_waveLabel = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
		_waveLabel->color = { 1.0f, 1.0f, 1.0f };
		_waveLabel->setText(
			"Wave: " + std::to_string(_wave.hudWaveIndex()) + "/" + std::to_string(WaveCount),
			24);

		_waveButton = std::make_unique<hl::ui::Button>(_layoutRoot->addChild(), *_typeface);
		_waveButton->setText("Start Wave (20)", 32);
		_waveButton->onClick = [this]() { startWave(); };

		_overlayRoot = std::make_unique<hl::ui::Node>();
		_overlayRoot->setFillParent();

		auto& dimNode = _overlayRoot->addChild();
		dimNode.setFillParent();
		_dim = std::make_unique<hl::ui::Panel>(dimNode);
		_dim->color = { 0.0f, 0.0f, 0.0f };
		_dim->opacity = 0.55f;
		_dim->hitTestEnabled = false;

		auto& column = _overlayRoot->addChild();
		column.kind = hl::ui::Kind::Column;
		column.gap = 24.0f;
		column.padding = { 32.0f, 24.0f, 32.0f, 24.0f };
		column.crossAlign = hl::ui::Align::Center;
		column.setCenter({ 0.0f, 0.0f });

		_overlayPanel = std::make_unique<hl::ui::Panel>(column);
		_overlayPanel->color = { 0.08f, 0.09f, 0.12f };
		_overlayPanel->hitTestEnabled = false;

		_overlayHeading = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
		_overlayHeading->color = { 1.0f, 0.5f, 0.0f };
		_overlayHeading->setText("Game Over", 64);

		_titleButton = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
		_titleButton->setText("Title", 48);
		_titleButton->onClick = [this]()
		{
			_sceneHost.goTitle();
		};
	}

	void TowerDefenseGameEngineScene::rebuildHud()
	{
		_goldLabel->setText("Gold: " + std::to_string(_gameState.gold()), 24);
		_livesLabel->setText("Lives: " + std::to_string(_gameState.lives()), 24);
		_waveLabel->setText(
			"Wave: " + std::to_string(_wave.hudWaveIndex()) + "/" + std::to_string(WaveCount),
			24);
		if (!_wave.inCombat() && !matchEnded())
		{
			_waveButton->setText(
				"Start Wave (" + std::to_string(_wave.buildSecondsRemaining()) + ")",
				32);
		}
		hl::ui::prepareTree(*_layoutRoot);

		const glm::vec2 goldSize = _goldLabel->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		_goldLabel->node().setTopLeft(goldSize);
		_goldLabel->node().relative = { 16.0f, 16.0f };
		_goldLabel->node().intrinsicSize.reset();

		const glm::vec2 livesSize = _livesLabel->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		_livesLabel->node().setTopLeft(livesSize);
		_livesLabel->node().relative = { 16.0f, 16.0f + goldSize.y + 8.0f };
		_livesLabel->node().intrinsicSize.reset();

		const glm::vec2 waveSize = _waveLabel->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
		_waveLabel->node().setTopLeft(waveSize);
		_waveLabel->node().relative = { 16.0f, 16.0f + goldSize.y + 8.0f + livesSize.y + 8.0f };
		_waveLabel->node().intrinsicSize.reset();

		if (!_wave.inCombat() && !matchEnded())
		{
			_waveButton->hitTestEnabled = true;
			const glm::vec2 buttonSize = _waveButton->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			_waveButton->node().setTopCenter(buttonSize);
			_waveButton->node().relative = { 0.0f, 16.0f };
			_waveButton->node().intrinsicSize.reset();
		}
		else
		{
			_waveButton->hitTestEnabled = false;
			_waveButton->node().setTopLeft({ 0.0f, 0.0f });
			_waveButton->node().relative = { -10000.0f, -10000.0f };
			_waveButton->node().intrinsicSize.reset();
		}

		const auto fb = _engine.getInputManager().getFramebufferSize();
		const hl::ui::Box screen{ 0.0f, 0.0f, fb.x, fb.y };
		hl::ui::layout(*_layoutRoot, screen);

		_uiBatch.begin();
		UiBatchPaint paint(_uiBatch);
		hl::ui::paintTree(*_layoutRoot, paint);

		if (!matchEnded())
		{
			hl::ui::dispatch(*_layoutRoot, readMenuPointer(_engine));
		}

		if (matchEnded())
		{
			hl::ui::prepareTree(*_overlayRoot);
			hl::ui::layout(*_overlayRoot, screen);
			hl::ui::dispatch(*_overlayRoot, readMenuPointer(_engine));
			hl::ui::paintTree(*_overlayRoot, paint);
		}
	}

	void TowerDefenseGameEngineScene::update(uint32_t /*currentFrame*/, float delta)
	{
		if (_invalidFlashRemaining > 0.0f)
		{
			_invalidFlashRemaining -= delta;
			if (_invalidFlashRemaining <= 0.0f)
			{
				_invalidFlashRemaining = 0.0f;
				_flashTower = nullptr;
				_flashGhost = false;
			}
		}

		if (!matchEnded())
		{
			tickWave(delta);
			_scene.update(delta);
			tryClearWave();
		}

		rebuildHud();
		updateGhost();
		updateCameraOrbit();
		updateCameraFollow(delta);

		if (!matchEnded())
		{
			tryHandleBoardClick();
		}
	}

	void TowerDefenseGameEngineScene::updateGpuResources(uint32_t currentFrame)
	{
		updateCameraOrbit();

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

	void TowerDefenseGameEngineScene::updateCameraOrbit()
	{
		const auto& input = _engine.getInputManager();
		auto* camera = boardCamera();
		if (camera == nullptr)
		{
			return;
		}

		if (!input.isButtonDown(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			_orbitDragging = false;
			return;
		}

		const float mouseX = input.getMousePosition().x;
		if (!_orbitDragging)
		{
			_orbitDragging = true;
			_orbitStartMouseX = mouseX;
			_orbitStartYaw = camera->getYaw();
		}

		const float yaw = _orbitStartYaw - (mouseX - _orbitStartMouseX) * CameraOrbitDegreesPerPixel;
		camera->setLookAtPose(glm::vec3(0.0f), yaw, camera->getPitch(), _cameraDistance);
	}

	void TowerDefenseGameEngineScene::updateCameraFollow(float delta)
	{
		if (_orbitDragging)
		{
			return;
		}

		auto* camera = boardCamera();
		if (camera == nullptr)
		{
			return;
		}

		const float follow = 1.0f - std::exp(-CameraSmooth * delta);
		_cameraDistance += (_cameraDistanceTarget - _cameraDistance) * follow;
		const glm::vec3 front = camera->getFront();
		if (glm::length(front) < 1e-4f)
		{
			return;
		}

		camera->setPosition(-glm::normalize(front) * _cameraDistance);
	}

	hl::Camera* TowerDefenseGameEngineScene::boardCamera() const
	{
		auto it = _cameras.find("Default");
		if (it == _cameras.end())
		{
			return nullptr;
		}

		return dynamic_cast<hl::Camera*>(it->second);
	}

	void TowerDefenseGameEngineScene::OnEvent(const hl::Event& event)
	{
		const auto* scroll = dynamic_cast<const hl::ScrollEvent*>(&event);
		if (scroll == nullptr)
		{
			return;
		}

		const float delta = static_cast<float>(scroll->getY()) * CameraZoomStep;
		_cameraDistanceTarget = std::clamp(
			_cameraDistanceTarget - delta,
			CameraDistanceMin,
			CameraDistanceMax);
	}
}
