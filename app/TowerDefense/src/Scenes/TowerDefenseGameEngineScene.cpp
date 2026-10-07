#include "Scenes/TowerDefenseGameEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include <AudioCatalog.hpp>
#include <GroundPick.hpp>
#include <SceneCatalog.hpp>
#include <SunUniformBufferObject.hpp>
#include <Components/PathFollowComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/CreepComponent.hpp>
#include <Components/StatusListComponent.hpp>
#include <Components/StatusRingComponent.hpp>
#include <Components/TileComponent.hpp>
#include <Components/TowerComponent.hpp>
#include <Components/BlockerComponent.hpp>
#include <Systems/PathFollowSystem.hpp>
#include <Systems/TowerFireSystem.hpp>
#include <Systems/ProjectileSystem.hpp>
#include <Systems/StatusSystem.hpp>
#include <Status.hpp>
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
#include <ranges>
#include <string>
#include <stdexcept>
#include <vector>

namespace tower
{

	TowerDefenseGameEngineScene::TowerDefenseGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		SceneHost& sceneHost,
		GameStateService& gameState,
		MatchContext& match,
		WaveService& wave,
		CreepCatalog& creeps,
		TowerCatalog& towers,
		WeaponCatalog& weapons,
		ProjectileCatalog& projectiles,
		StatusCatalog& statuses,
		StatusCategoryCatalog& statusCategories,
		EntityCatalog& entities,
		LevelCatalog& level,
		hl::audio::Audio& audio
	) :
		EngineScene(engine),
		_sceneHost(sceneHost),
		_engineConfig(engineConfig),
		_gameState(gameState),
		_match(match),
		_wave(wave),
		_creeps(creeps),
		_towers(towers),
		_weapons(weapons),
		_projectiles(projectiles),
		_statuses(statuses),
		_statusCategories(statusCategories),
		_entities(entities),
		_level(level),
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
		for (const auto& def : _towers.all())
		{
			resourceManager.Load<hl::ModelResource>(def.model, resourceContext);
		}
		for (const auto& def : _entities.all())
		{
			resourceManager.Load<hl::ModelResource>(def.model, resourceContext);
		}
		for (const auto& def : _creeps.all())
		{
			resourceManager.Load<hl::ModelResource>(def.model, resourceContext);
		}
		for (const auto& def : _projectiles.all())
		{
			resourceManager.Load<hl::ModelResource>(def.model, resourceContext);
		}
		spawnBlockers(resourceManager, resourceContext);
		spawnGhost(resourceManager);
		spawnRangeRing(resourceManager, resourceContext);
		spawnPathRibbons(resourceManager, resourceContext);

		auto* statuses = new StatusSystem(_scene, _statuses);
		statuses->onKill = [this]() { onCreepKilled(); };
		_scene.addSystem(statuses);
		auto* pathFollow = new PathFollowSystem(_scene, _level);
		pathFollow->onLeak = [this]() { onCreepLeaked(); };
		_scene.addSystem(pathFollow);
		_scene.addSystem(new TowerFireSystem(
			_scene,
			resourceManager,
			_towers,
			_weapons,
			_projectiles,
			_level,
			_match));
		auto* projectiles = new ProjectileSystem(_scene, _statuses, _statusCategories);
		projectiles->onKill = [this]() { onCreepKilled(); };
		_scene.addSystem(projectiles);
	}

	void TowerDefenseGameEngineScene::spawnBoard(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext)
	{
		auto black = resourceManager.Load<hl::ModelResource>(TileBlackModelId, resourceContext);
		auto white = resourceManager.Load<hl::ModelResource>(TileWhiteModelId, resourceContext);

		for (int tz = 0; tz < _level.boardDepth(); ++tz)
		{
			for (int tx = 0; tx < _level.boardWidth(); ++tx)
			{
				const bool dark = ((tx + tz) & 1) != 0;
				auto* entity = _scene.addEntity();
				entity->AddTag(TileTag);
				if (_level.isPathTile(tx, tz))
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
				tile->path = _level.isPathTile(tx, tz);

				entity->AddComponent<hl::TransformComponent>()->SetPosition(_level.tileCenter(tx, tz));
				entity->AddComponent<hl::ModelComponent>()->setModelId(
					dark ? black->GetId() : white->GetId());
			}
		}
	}

	void TowerDefenseGameEngineScene::spawnBlockers(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext&)
	{
		for (const auto& placement : _level.entities())
		{
			const auto* def = _entities.find(placement.id);
			if (def == nullptr)
			{
				throw std::runtime_error(
					std::string("Unknown blocker entity id '") + placement.id + "'");
			}

			auto* model = resourceManager.GetResource<hl::ModelResource>(def->model);
			auto* entity = _scene.addEntity();
			entity->AddTag(BlockerTag);
			auto* blocker = entity->AddComponent<BlockerComponent>();
			blocker->x = placement.x;
			blocker->z = placement.z;
			blocker->sizeX = def->sizeX;
			blocker->sizeZ = def->sizeZ;
			const auto minCorner = _level.tileCenter(placement.x, placement.z);
			const auto maxCorner = _level.tileCenter(
				placement.x + def->sizeX - 1,
				placement.z + def->sizeZ - 1);
			auto* transform = entity->AddComponent<hl::TransformComponent>();
			transform->SetPosition((minCorner + maxCorner) * 0.5f);
			transform->SetScale(glm::vec3(
				static_cast<float>(def->sizeX),
				1.0f,
				static_cast<float>(def->sizeZ)));
			entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		}
	}

	void TowerDefenseGameEngineScene::spawnTower(
		hl::ResourceManager& resourceManager,
		int tx,
		int tz,
		const std::string& defId)
	{
		const auto* def = _towers.find(defId);
		if (def == nullptr)
		{
			return;
		}

		auto* model = resourceManager.GetResource<hl::ModelResource>(def->model);
		auto* entity = _scene.addEntity();
		entity->AddTag(TowerTag);
		auto* tower = entity->AddComponent<TowerComponent>();
		tower->x = tx;
		tower->z = tz;
		tower->defId = defId;
		tower->slotCooldown.assign(def->weapons.size(), 0.0f);
		entity->AddComponent<hl::TransformComponent>()->SetPosition(_level.tileCenter(tx, tz));
		entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		_audio.play(CuePlace);
	}

	void TowerDefenseGameEngineScene::selectPlaceTool(const std::string& defId)
	{
		if (std::ranges::find(_placeableIds, defId) == _placeableIds.end())
		{
			return;
		}
		_selectedTowerId = _selectedTowerId == defId ? std::string{} : defId;
	}

	const TowerDef* TowerDefenseGameEngineScene::selectedTowerDef() const
	{
		if (_selectedTowerId.empty())
		{
			return nullptr;
		}

		return _towers.find(_selectedTowerId);
	}

	bool TowerDefenseGameEngineScene::isOccupied(int tx, int tz) const
	{
		return towerAt(tx, tz) != nullptr || blockerAt(tx, tz) != nullptr;
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

	hl::Entity* TowerDefenseGameEngineScene::blockerAt(int tx, int tz) const
	{
		for (auto* entity : _scene.getEntitiesByTag(BlockerTag))
		{
			const auto* blocker = entity->GetComponent<BlockerComponent>();
			if (blocker != nullptr
				&& tx >= blocker->x && tx < blocker->x + blocker->sizeX
				&& tz >= blocker->z && tz < blocker->z + blocker->sizeZ)
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
		const auto& def = _wave.nextCreep();
		auto* model = _resourceManager->GetResource<hl::ModelResource>(def.model);
		const auto start = _level.path().front();
		auto* entity = _scene.addEntity();
		entity->AddTag(CreepTag);
		auto* transform = entity->AddComponent<hl::TransformComponent>();
		transform->SetPosition(_level.tileCenter(start.x, start.z));
		transform->SetScale(glm::vec3(def.scale));
		entity->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
		auto* follow = entity->AddComponent<PathFollowComponent>();
		follow->fromIndex = 0;
		follow->t = 0.0f;
		follow->baseSpeed = def.speed;
		follow->speed = def.speed;
		auto* creep = entity->AddComponent<CreepComponent>();
		creep->resist = def.resist;
		creep->baseHealth = def.health;
		creep->baseSpeed = def.speed;
		creep->scale = def.scale;
		entity->AddComponent<StatusListComponent>();
		auto* health = entity->AddComponent<HealthComponent>();
		health->max = def.health;
		health->current = def.health;
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

		auto* model = resourceManager.GetResource<hl::ModelResource>(_towers.all().front().model);
		_ghost = _scene.addEntity();
		_ghost->AddTag(GhostTag);
		_ghost->AddComponent<hl::TransformComponent>()->SetPosition(_level.tileCenter(0, 0));
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
		transform->SetPosition(_level.tileCenter(0, 0, RangeRingY));
		transform->SetScale(glm::vec3(_towers.all().front().range, 1.0f, _towers.all().front().range));
		_rangeRing->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
	}

	void TowerDefenseGameEngineScene::syncStatusRings()
	{
		if (_resourceManager == nullptr)
		{
			return;
		}

		struct Desired
		{
			int creepId = 0;
			StatusRing ring;
			glm::vec3 position{ 0.0f };
		};

		std::vector<Desired> desired;
		for (auto* creep : _scene.getEntitiesWithComponents<
			hl::TransformComponent,
			CreepComponent,
			StatusListComponent>(CreepTag))
		{
			if (_scene.isPendingRemoval(creep->Id))
			{
				continue;
			}

			const auto* list = creep->GetComponent<StatusListComponent>();
			const auto* creepComp = creep->GetComponent<CreepComponent>();
			const auto pos = creep->GetComponent<hl::TransformComponent>()->GetPosition();
			const auto rings = statusRings(
				list->instances,
				_statuses,
				_statusCategories,
				creepComp->scale);
			for (const auto& ring : rings)
			{
				desired.push_back(Desired{
					.creepId = creep->Id,
					.ring = ring,
					.position = glm::vec3(pos.x, StatusRingY, pos.z)
				});
			}
		}

		for (auto* entity : _scene.getEntitiesByTag(StatusRingTag))
		{
			if (_scene.isPendingRemoval(entity->Id))
			{
				continue;
			}

			auto* ring = entity->GetComponent<StatusRingComponent>();
			const bool keep = ring != nullptr && std::any_of(
				desired.begin(),
				desired.end(),
				[&](const Desired& item)
				{
					return item.creepId == ring->creepId && item.ring.categoryId == ring->categoryId;
				});
			if (!keep)
			{
				_scene.removeEntity(entity->Id);
			}
		}

		auto* model = _resourceManager->GetResource<hl::ModelResource>(RangeRingModelId);
		if (model == nullptr)
		{
			return;
		}

		for (const auto& item : desired)
		{
			hl::Entity* existing = nullptr;
			for (auto* entity : _scene.getEntitiesByTag(StatusRingTag))
			{
				if (_scene.isPendingRemoval(entity->Id))
				{
					continue;
				}

				auto* ring = entity->GetComponent<StatusRingComponent>();
				if (ring != nullptr
					&& ring->creepId == item.creepId
					&& ring->categoryId == item.ring.categoryId)
				{
					existing = entity;
					break;
				}
			}

			const std::string materialName = "status_ring_" + item.ring.categoryId;
			_engine.getMaterialSystem().addMaterial(hl::Material{
				.name = materialName,
				.diffuse = item.ring.color
			});

			if (existing == nullptr)
			{
				existing = _scene.addEntity();
				existing->AddTag(StatusRingTag);
				existing->AddComponent<hl::TransformComponent>();
				existing->AddComponent<hl::ModelComponent>()->setModelId(model->GetId());
				auto* ring = existing->AddComponent<StatusRingComponent>();
				ring->creepId = item.creepId;
				ring->categoryId = item.ring.categoryId;
				ring->materialName = materialName;
			}

			auto* transform = existing->GetComponent<hl::TransformComponent>();
			transform->SetPosition(item.position);
			transform->SetScale(glm::vec3(item.ring.scaleXZ, 1.0f, item.ring.scaleXZ));
			existing->GetComponent<StatusRingComponent>()->materialName = materialName;
		}
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

		const auto& path = _level.path();
		for (std::size_t i = 0; i + 1 < path.size(); ++i)
		{
			const glm::vec3 a = _level.tileCenter(path[i].x, path[i].z, PathRibbonY);
			const glm::vec3 b = _level.tileCenter(path[i + 1].x, path[i + 1].z, PathRibbonY);
			const glm::vec3 delta = b - a;
			const float length = glm::length(delta);
			if (length < 1e-6f)
			{
				continue;
			}

			auto* entity = _scene.addEntity();
			entity->AddTag(PathRibbonTag);
			auto* transform = entity->AddComponent<hl::TransformComponent>();
			transform->SetPosition((a + b) * 0.5f);
			transform->SetRotation(glm::vec3(0.0f, glm::degrees(std::atan2(delta.x, delta.z)), 0.0f));
			transform->SetScale(glm::vec3(PathRibbonWidth, 1.0f, length));
			entity->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
		}
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

		return _level.worldToTile(*hit);
	}

	void TowerDefenseGameEngineScene::updateGhost()
	{
		_ghostVisible = false;
		_ghostPlaceable = false;
		const auto* def = selectedTowerDef();
		if (_ghost == nullptr
			|| def == nullptr
			|| matchEnded()
			|| _wave.inCombat()
			|| uiBlocksBoardClick())
		{
			return;
		}

		auto* model = _resourceManager->GetResource<hl::ModelResource>(def->model);
		_ghost->GetComponent<hl::ModelComponent>()->setModelId(model->GetId());

		const auto tile = hoveredTile();
		if (!tile || towerAt(tile->x, tile->z) != nullptr)
		{
			return;
		}

		_ghost->GetComponent<hl::TransformComponent>()->SetPosition(
			_level.tileCenter(tile->x, tile->z));
		if (_inspectTower == nullptr && _rangeRing != nullptr)
		{
			auto* ring = _rangeRing->GetComponent<hl::TransformComponent>();
			ring->SetPosition(_level.tileCenter(tile->x, tile->z, RangeRingY));
			ring->SetScale(glm::vec3(def->range, 1.0f, def->range));
		}
		_ghostPlaceable = !_level.isPathTile(tile->x, tile->z)
			&& blockerAt(tile->x, tile->z) == nullptr
			&& _gameState.gold() >= def->cost;
		_ghostVisible = true;
	}

	void TowerDefenseGameEngineScene::clearInspect()
	{
		_inspectTower = nullptr;
	}

	void TowerDefenseGameEngineScene::setInspect(hl::Entity* tower)
	{
		_inspectTower = tower;
	}

	void TowerDefenseGameEngineScene::sellInspectedTower()
	{
		if (_inspectTower == nullptr)
		{
			return;
		}

		const auto* tower = _inspectTower->GetComponent<TowerComponent>();
		const auto* def = (tower != nullptr) ? _towers.find(tower->defId) : nullptr;
		const int cost = def != nullptr ? def->cost : _towers.all().front().cost;
		_gameState.addGold(towerSellRefund(cost));
		if (_flashTower == _inspectTower)
		{
			_flashTower = nullptr;
			_invalidFlashRemaining = 0.0f;
			_flashGhost = false;
		}
		_scene.removeEntity(_inspectTower->Id);
		clearInspect();
	}

	void TowerDefenseGameEngineScene::syncInspectRing()
	{
		if (_inspectTower == nullptr || _rangeRing == nullptr)
		{
			return;
		}

		const auto* tower = _inspectTower->GetComponent<TowerComponent>();
		if (tower == nullptr)
		{
			clearInspect();
			return;
		}

		auto* ring = _rangeRing->GetComponent<hl::TransformComponent>();
		ring->SetPosition(_level.tileCenter(tower->x, tower->z, RangeRingY));
		const auto* def = _towers.find(tower->defId);
		const float range = def != nullptr ? def->range : _towers.all().front().range;
		ring->SetScale(glm::vec3(range, 1.0f, range));
	}

	void TowerDefenseGameEngineScene::tryHandleBoardClick()
	{
		if (matchEnded() || uiBlocksBoardClick())
		{
			return;
		}
		const auto& input = _engine.getInputManager();
		const bool released = input.isButtonReleased(GLFW_MOUSE_BUTTON_1);
		if (!released)
		{
			return;
		}

		const auto tile = hoveredTile();
		if (!tile)
		{
			return;
		}

		if (_wave.inCombat())
		{
			return;
		}

		if (auto* tower = towerAt(tile->x, tile->z))
		{
			setInspect(tower);
			return;
		}

		if (_inspectTower != nullptr)
		{
			clearInspect();
		}

		const auto* def = selectedTowerDef();
		if (def == nullptr)
		{
			return;
		}

		if (blockerAt(tile->x, tile->z) != nullptr)
		{
			flashInvalid(tile->x, tile->z);
			return;
		}

		if (_level.isPathTile(tile->x, tile->z) || !_gameState.trySpend(def->cost))
		{
			flashInvalid(tile->x, tile->z);
			return;
		}

		spawnTower(*_resourceManager, tile->x, tile->z, def->id);
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
						|| entity->HasTag(StatusRingTag)
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
						pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(
							mesh.materialName);
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
					if (_inspectTower == nullptr)
					{
						drawGhostLit(_rangeRing, ghostMaterial);
					}
				}

				if (_inspectTower != nullptr)
				{
					const uint32_t okMaterial = _engine.getMaterialSystem().getMaterialIndex(GhostOkMaterial);
					drawGhostLit(_rangeRing, okMaterial);
				}

				if (_flashTower != nullptr && _invalidFlashRemaining > 0.0f && blinkOn)
				{
					drawGhostLit(_flashTower, badMaterial);
				}

				for (auto* entity : _scene.getEntitiesByTag(StatusRingTag))
				{
					if (_scene.isPendingRemoval(entity->Id))
					{
						continue;
					}

					auto* ring = entity->GetComponent<StatusRingComponent>();
					if (ring == nullptr)
					{
						continue;
					}

					drawGhostLit(
						entity,
						_engine.getMaterialSystem().getMaterialIndex(ring->materialName));
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
		clearInspect();
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
		if (_match.campaign)
		{
			_sceneHost.onCampaignWon(_match.nodeId);
		}
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
		_gameState.addGold(_level.killGold());
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
			"Wave: " + std::to_string(_wave.hudWaveIndex()) + "/" + std::to_string(_level.waveCount()),
			24);

		_waveButton = std::make_unique<hl::ui::Button>(_layoutRoot->addChild(), *_typeface);
		_waveButton->setText("Start Wave (20)", 32);
		_waveButton->onClick = [this]() { startWave(); };

		_buildBarPanel = std::make_unique<hl::ui::Panel>(_layoutRoot->addChild());
		_buildBarPanel->color = { 0.08f, 0.09f, 0.12f };
		_buildBarPanel->opacity = 0.92f;
		std::vector<std::string> catalogOrder;
		catalogOrder.reserve(_towers.all().size());
		for (const auto& def : _towers.all())
		{
			catalogOrder.push_back(def.id);
		}
		_placeableIds = matchPlaceableTowerIds(catalogOrder, _match);
		for (const auto& id : _placeableIds)
		{
			const auto* def = _towers.find(id);
			if (def == nullptr)
			{
				continue;
			}
			auto button = std::make_unique<hl::ui::Button>(
				_buildBarPanel->node().addChild(),
				*_typeface);
			button->setText(
				def->label + " (" + std::to_string(def->cost) + ")",
				24);
			button->onClick = [this, id]() { selectPlaceTool(id); };
			_placeButtons.push_back(std::move(button));
		}

		_inspectPanel = std::make_unique<hl::ui::Panel>(_layoutRoot->addChild());
		_inspectPanel->color = { 0.08f, 0.09f, 0.12f };
		_inspectPanel->opacity = 0.92f;
		_inspectLabel = std::make_unique<hl::ui::Label>(_inspectPanel->node().addChild(), *_typeface);
		_inspectLabel->color = { 1.0f, 1.0f, 1.0f };
		_inspectLabel->setText(_towers.all().front().label, 24);
		_sellButton = std::make_unique<hl::ui::Button>(_inspectPanel->node().addChild(), *_typeface);
		_sellButton->setText(
			"Sell (" + std::to_string(towerSellRefund(_towers.all().front().cost)) + ")",
			24);
		_sellButton->onClick = [this]() { sellInspectedTower(); };

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
		if (_match.campaign)
		{
			_titleButton->setText("Hub", 48);
			_titleButton->onClick = [this]()
			{
				_sceneHost.goCampaignHub();
			};
		}
		else
		{
			_titleButton->setText("Title", 48);
			_titleButton->onClick = [this]()
			{
				_sceneHost.goTitle();
			};
		}
	}

	void TowerDefenseGameEngineScene::rebuildHud()
	{
		_goldLabel->setText("Gold: " + std::to_string(_gameState.gold()), 24);
		_livesLabel->setText("Lives: " + std::to_string(_gameState.lives()), 24);
		_waveLabel->setText(
			"Wave: " + std::to_string(_wave.hudWaveIndex()) + "/" + std::to_string(_level.waveCount()),
			24);
		if (!_wave.inCombat() && !matchEnded())
		{
			_waveButton->setText(
				"Start Wave (" + std::to_string(_wave.buildSecondsRemaining()) + ")",
				32);
		}
		for (std::size_t i = 0; i < _placeButtons.size(); ++i)
		{
			const auto* def = _towers.find(_placeableIds[i]);
			if (def == nullptr)
			{
				continue;
			}
			_placeButtons[i]->setText(
				def->label + " (" + std::to_string(def->cost) + ")",
				24);
		}
		if (_inspectTower != nullptr)
		{
			const auto* tower = _inspectTower->GetComponent<TowerComponent>();
			const auto* def = (tower != nullptr) ? _towers.find(tower->defId) : nullptr;
			if (def != nullptr)
			{
				_inspectLabel->setText(def->label, 24);
				_sellButton->setText(
					"Sell (" + std::to_string(towerSellRefund(def->cost)) + ")",
					24);
			}
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

		constexpr glm::vec2 barPad{ 16.0f, 8.0f };
		constexpr float chipGap = 8.0f;
		std::vector<glm::vec2> chipSizes(_placeButtons.size());
		float chipsWidth = 0.0f;
		float chipsHeight = 0.0f;
		bool anySelected = false;
		for (std::size_t i = 0; i < _placeButtons.size(); ++i)
		{
			const auto* def = _towers.find(_placeableIds[i]);
			if (def == nullptr)
			{
				continue;
			}
			chipSizes[i] = _placeButtons[i]->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			chipsWidth += chipSizes[i].x;
			chipsHeight = std::max(chipsHeight, chipSizes[i].y);
			const bool available =
				!matchEnded() && !_wave.inCombat() && _gameState.gold() >= def->cost;
			const bool selected = _selectedTowerId == def->id;
			anySelected = anySelected || selected;
			_placeButtons[i]->color = selected
				? glm::vec3{ 1.0f, 1.0f, 1.0f }
				: glm::vec3{ 0.55f, 0.55f, 0.55f };
			_placeButtons[i]->hitTestEnabled = available;
		}
		_buildBarPanel->color = anySelected
			? glm::vec3{ 0.28f, 0.38f, 0.28f }
			: glm::vec3{ 0.08f, 0.09f, 0.12f };
		_buildBarPanel->hitTestEnabled = !matchEnded();
		if (!_placeButtons.empty())
		{
			chipsWidth += chipGap * static_cast<float>(_placeButtons.size() - 1);
		}
		_buildBarPanel->node().setBottomCenter(
			{ chipsWidth + barPad.x * 2.0f, chipsHeight + barPad.y * 2.0f });
		_buildBarPanel->node().relative = { 0.0f, 0.0f };
		float chipX = barPad.x;
		for (std::size_t i = 0; i < _placeButtons.size(); ++i)
		{
			_placeButtons[i]->node().setTopLeft(chipSizes[i]);
			_placeButtons[i]->node().relative = { chipX, barPad.y };
			_placeButtons[i]->node().intrinsicSize.reset();
			chipX += chipSizes[i].x + chipGap;
		}

		const bool showInspect = _inspectTower != nullptr && !_wave.inCombat() && !matchEnded();
		if (showInspect)
		{
			_inspectPanel->hitTestEnabled = true;
			_sellButton->hitTestEnabled = true;
			const glm::vec2 titleSize = _inspectLabel->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			const glm::vec2 sellSize = _sellButton->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
			constexpr glm::vec2 inspectPad{ 16.0f, 12.0f };
			constexpr float inspectGap = 8.0f;
			_inspectPanel->node().setBottomRight({
				std::max(titleSize.x, sellSize.x) + inspectPad.x * 2.0f,
				titleSize.y + inspectGap + sellSize.y + inspectPad.y * 2.0f
			});
			_inspectPanel->node().relative = { 0.0f, 0.0f };
			_inspectLabel->node().setTopLeft(titleSize);
			_inspectLabel->node().relative = inspectPad;
			_inspectLabel->node().intrinsicSize.reset();
			_sellButton->node().setTopLeft(sellSize);
			_sellButton->node().relative = { inspectPad.x, inspectPad.y + titleSize.y + inspectGap };
			_sellButton->node().intrinsicSize.reset();
		}
		else
		{
			_inspectPanel->hitTestEnabled = false;
			_sellButton->hitTestEnabled = false;
			_inspectPanel->node().setTopLeft({ 0.0f, 0.0f });
			_inspectPanel->node().relative = { -10000.0f, -10000.0f };
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

		if (matchEnded() || _wave.inCombat())
		{
			clearInspect();
		}

		if (_engine.getInputManager().isKeyReleased(GLFW_KEY_ESCAPE))
		{
			clearInspect();
			_selectedTowerId.clear();
		}

		rebuildHud();
		updateGhost();
		syncInspectRing();
		syncStatusRings();
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
