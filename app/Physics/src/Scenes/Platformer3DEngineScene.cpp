#include <Scenes/Platformer3DEngineScene.hpp>
#include <Components/PhysicsLinkComponent.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <Scenes/TitleEngineScene.hpp>
#include <SunUniformBufferObject.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Resource/Material.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/PipelineDrawData.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include <stdexcept>

namespace phys
{
	namespace
	{
		constexpr float kWalkSpeed = 6.5f;
		constexpr float kClimbSpeed = 5.5f;
		constexpr float kAirControl = 0.7f;
		constexpr float kCapsuleRadius = 0.4f;
		constexpr float kCapsuleHeight = 1.2f;
		constexpr float kOrbitDegreesPerPixel = 0.25f;
		constexpr float kOrbitYawSpeed = 90.f;
		constexpr float kMinPitch = -80.f;
		constexpr float kMaxPitch = -5.f;

		glm::vec3 flattenXz(glm::vec3 v)
		{
			v.y = 0.f;
			const float len = glm::length(v);
			if (len < 1e-5f)
			{
				return glm::vec3(0.f);
			}
			return v / len;
		}
	}

	Platformer3DEngineScene::Platformer3DEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		hl::physics::Context& physicsContext)
		: EngineScene(engine)
		, _engineConfig(engineConfig)
		, _physicsContext(physicsContext)
		, _viewportWidth(static_cast<int>(engineConfig.Width))
		, _viewportHeight(static_cast<int>(engineConfig.Height))
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_worldCamera = new hl::Camera(
			glm::vec3(0.f, 4.f, 10.f),
			glm::vec3(0.f, 1.f, 0.f),
			_orbitYaw,
			_orbitPitch);
		_cameras.insert({ "World", _worldCamera });
		_engine.getEventBus().AddListener(this);

		hl::physics::WorldSettings settings;
		settings.dim = hl::physics::Dim::D3;
		settings.gravity = glm::vec3(0.f, -32.f, 0.f);
		_world = std::make_unique<hl::physics::World>(_physicsContext, settings);
		_world->setContactCallback([this](const hl::physics::Contact& contact)
		{
			if (!_ladder.valid())
			{
				return;
			}
			const bool involvesLadder = contact.bodyA == _ladder || contact.bodyB == _ladder;
			if (!involvesLadder)
			{
				return;
			}
			const hl::physics::BodyId other = contact.bodyA == _ladder ? contact.bodyB : contact.bodyA;
			if (isCrate(other))
			{
				return;
			}
			_ladderContactThisStep = true;
		});
	}

	Platformer3DEngineScene::~Platformer3DEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void Platformer3DEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
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
					.resource = cameraMatrixResourceId,
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
					.resource = hl::MaterialSystem::AlbedoAtlasName,
					.count = static_cast<uint32_t>(MAX_MATERIAL_TEXTURES),
					.updateFrequency = hl::DescriptorUpdateFrequency::Static,
					.partiallyBound = true,
					.updateAfterBind = true
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

		std::vector<hl::RenderpassInfo> renderpasses{
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
						}
					}
				}
			},
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
		_sunUbo = resourceManager.Load<hl::UniformBufferResource>(
			"sun_ubo",
			resourceContext,
			sizeof(SunUniformBufferObject),
			MAX_FRAMES_IN_FLIGHT,
			1);
		resourceManager.Load<hl::ModelResource>("cube", resourceContext);

		buildCourse();
		addTextEntity(
			_scene,
			_engine,
			"hint",
			"WASD walk  RMB/QE orbit  Space jump (double)  Up/Down climb  Escape: title",
			36);
		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);

		registerPipelineDraw(
			"model_pipeline",
			[this](hl::PipelineDrawData& pdd)
			{
				const auto cameraIndex = static_cast<uint32_t>(getCameraIndex("World"));
				for (const auto& entity : pdd.scene->getEntities())
				{
					if (!entity->HasComponents<hl::TransformComponent, hl::ModelComponent, PhysicsLinkComponent>())
					{
						continue;
					}

					const auto* transform = entity->GetComponent<hl::TransformComponent>();
					const auto* model = entity->GetComponent<hl::ModelComponent>();
					const auto* link = entity->GetComponent<PhysicsLinkComponent>();
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
					if (!link->materialName.empty())
					{
						pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(link->materialName);
					}

					for (const auto& mesh : modelResource->getMeshes())
					{
						if (link->materialName.empty())
						{
							pc.materialIndex = _engine.getMaterialSystem().getMaterialIndex(mesh.materialName);
						}
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
	}

	void Platformer3DEngineScene::update(uint32_t currentFrame, float delta)
	{
		if (_world == nullptr || !_player.valid())
		{
			return;
		}

		applyInput(delta);
		_ladderContactThisStep = false;
		_world->step(delta);
		if (_world->isGrounded(_player))
		{
			_airJumpsLeft = 1;
		}
		_ladderOverlap = _ladderContactThisStep || ladderAabbOverlapsCharacter();
		syncTransformsFromWorld();

		if (_sunUbo)
		{
			SunUniformBufferObject ubo{};
			const auto dir = glm::normalize(glm::vec3(0.45f, 0.85f, 0.30f));
			ubo.direction = glm::vec4(dir, 1.0f);
			ubo.color = glm::vec4(1.0f, 0.97f, 0.90f, 0.35f);
			ubo.ambient = glm::vec4(0.18f, 0.18f, 0.18f, 0.0f);
			_sunUbo.Get()->getUniformBuffer(currentFrame).writeToBuffer(&ubo, 0);
		}
	}

	void Platformer3DEngineScene::additionalCleanup()
	{
		_world.reset();
		_player = hl::physics::CharacterId::invalid();
		_ladder = hl::physics::BodyId::invalid();
		_crates.clear();
		_ladderOverlap = false;
		_ladderContactThisStep = false;
		_jumpQueued = false;
		_airJumpsLeft = 1;
	}

	void Platformer3DEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_ESCAPE)
			{
				_engine.setScene(new TitleEngineScene(_engine, _engineConfig, _physicsContext));
			}
			else if (ke->GetKeyCode() == GLFW_KEY_SPACE)
			{
				_jumpQueued = true;
			}
		}
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void Platformer3DEngineScene::handleWindowSizeChange(int width, int height)
	{
		_viewportWidth = width;
		_viewportHeight = height;
		centerTextAt(_scene, _engine, width, height, "hint", static_cast<float>(-height) / 3.0f + 40.0f);
	}

	hl::Entity* Platformer3DEngineScene::addVisual(
		const std::string& name,
		const glm::vec3& visualHalfExtents,
		const glm::vec4& color,
		bool isCharacter)
	{
		auto entity = _scene.addEntity(name);
		entity->AddTag("PHYSICS");
		entity->AddComponent<hl::TransformComponent>();
		entity->AddComponent<hl::ModelComponent>()->setModelId("cube");
		auto link = entity->AddComponent<PhysicsLinkComponent>();
		link->visualHalfExtents = visualHalfExtents;
		link->color = color;
		link->materialName = name;
		link->isCharacter = isCharacter;
		_engine.getMaterialSystem().addMaterial(hl::Material{
			.name = name,
			.diffuse = glm::vec3(color),
			.diffuseTex = hl::MaterialSystem::FallbackTextureName
		});
		return entity;
	}

	void Platformer3DEngineScene::buildCourse()
	{
		const auto addStatic = [this](
			const std::string& name,
			glm::vec3 halfExtents,
			glm::vec3 position,
			glm::quat rotation,
			glm::vec4 color,
			bool oneWay = false)
		{
			auto* entity = addVisual(name, halfExtents, color, false);
			hl::physics::BodyDesc desc;
			desc.shape.kind = hl::physics::Shape::Kind::Box;
			desc.shape.halfExtents = halfExtents;
			desc.motion = hl::physics::MotionType::Static;
			desc.layer = hl::physics::Layer::NonMoving;
			desc.pose.position = position;
			desc.pose.rotation = rotation;
			desc.oneWay = oneWay;
			desc.userData = static_cast<std::uint64_t>(entity->Id);
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->body = _world->createBody(desc);
			if (!link->body.valid())
			{
				throw std::runtime_error("Platformer3DEngineScene: failed to create static body");
			}
			return link->body;
		};

		addStatic(
			"floor",
			{ 12.f, 0.5f, 4.f },
			{ 10.f, -0.5f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.35f, 0.38f, 1.f));

		const glm::quat gentle = glm::angleAxis(glm::radians(25.f), glm::vec3(0.f, 0.f, 1.f));
		addStatic(
			"ramp",
			{ 4.f, 0.2f, 1.5f },
			{ 5.f, 1.9f, 0.f },
			gentle,
			glm::vec4(0.55f, 0.75f, 0.95f, 1.f));

		addStatic(
			"platform_low",
			{ 1.5f, 0.2f, 1.5f },
			{ 8.5f, 1.5f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.85f, 0.9f, 1.f),
			true);
		addStatic(
			"platform_mid",
			{ 1.5f, 0.2f, 1.5f },
			{ 9.f, 2.8f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.85f, 0.9f, 1.f),
			true);

		addStatic(
			"platform_exit",
			{ 2.f, 0.2f, 1.5f },
			{ 14.2f, 4.2f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.7f, 0.7f, 0.72f, 1.f));

		const glm::quat steep = glm::angleAxis(glm::radians(70.f), glm::vec3(0.f, 0.f, 1.f));
		addStatic(
			"steep",
			{ 2.f, 0.2f, 1.5f },
			{ 18.f, 1.2f, 0.f },
			steep,
			glm::vec4(0.85f, 0.3f, 0.25f, 1.f));

		{
			_ladderPosition = { 12.f, 2.f, 0.f };
			_ladderHalfExtents = { 0.25f, 2.f, 0.4f };
			auto* entity = addVisual("ladder", _ladderHalfExtents, glm::vec4(0.25f, 0.85f, 0.4f, 1.f), false);
			hl::physics::BodyDesc desc;
			desc.shape.kind = hl::physics::Shape::Kind::Box;
			desc.shape.halfExtents = _ladderHalfExtents;
			desc.motion = hl::physics::MotionType::Static;
			desc.layer = hl::physics::Layer::Sensor;
			desc.sensor = true;
			desc.pose.position = _ladderPosition;
			desc.userData = static_cast<std::uint64_t>(entity->Id);
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->body = _world->createBody(desc);
			_ladder = link->body;
			if (!_ladder.valid())
			{
				throw std::runtime_error("Platformer3DEngineScene: failed to create ladder sensor");
			}
		}

		{
			auto* entity = addVisual("crate", { 0.25f, 0.25f, 0.25f }, glm::vec4(0.72f, 0.5f, 0.22f, 1.f), false);
			hl::physics::BodyDesc desc;
			desc.shape.kind = hl::physics::Shape::Kind::Box;
			desc.shape.halfExtents = { 0.25f, 0.25f, 0.25f };
			desc.motion = hl::physics::MotionType::Dynamic;
			desc.layer = hl::physics::Layer::Moving;
			desc.pose.position = { 4.f, 1.f, 0.f };
			desc.mass = 2.f;
			desc.userData = static_cast<std::uint64_t>(entity->Id);
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->body = _world->createBody(desc);
			if (!link->body.valid())
			{
				throw std::runtime_error("Platformer3DEngineScene: failed to create crate");
			}
			_crates.push_back(link->body);
		}

		{
			const glm::vec3 playerHalf{
				kCapsuleRadius,
				(kCapsuleHeight + 2.f * kCapsuleRadius) * 0.5f,
				kCapsuleRadius
			};
			auto* entity = addVisual("player", playerHalf, glm::vec4(0.95f, 0.85f, 0.2f, 1.f), true);
			hl::physics::CharacterDesc desc;
			desc.dim = hl::physics::Dim::D3;
			desc.pose.position = { 1.f, 2.f, 0.f };
			desc.capsuleRadius = kCapsuleRadius;
			desc.capsuleHeight = kCapsuleHeight;
			desc.maxSlopeAngleDeg = 45.f;
			desc.jumpSpeed = 11.f;
			_player = _world->createCharacter(desc);
			if (!_player.valid())
			{
				throw std::runtime_error("Platformer3DEngineScene: failed to create character");
			}
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->character = _player;
		}
	}

	void Platformer3DEngineScene::applyInput(float delta)
	{
		updateOrbit(delta);

		auto& input = _engine.getInputManager();
		glm::vec3 wish{ 0.f };
		const glm::vec3 forward = flattenXz(_worldCamera->getFront());
		const glm::vec3 right = flattenXz(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f)));
		if (input.isKeyDown(GLFW_KEY_W))
		{
			wish += forward * kWalkSpeed;
		}
		if (input.isKeyDown(GLFW_KEY_S))
		{
			wish -= forward * kWalkSpeed;
		}
		if (input.isKeyDown(GLFW_KEY_A))
		{
			wish -= right * kWalkSpeed;
		}
		if (input.isKeyDown(GLFW_KEY_D))
		{
			wish += right * kWalkSpeed;
		}

		const bool climbUp = input.isKeyDown(GLFW_KEY_UP) || input.isKeyDown(GLFW_KEY_R);
		const bool climbDown = input.isKeyDown(GLFW_KEY_DOWN) || input.isKeyDown(GLFW_KEY_F);
		if (_ladderOverlap && (climbUp || climbDown))
		{
			_world->setClimbing(_player, true);
			if (climbUp)
			{
				wish.y += kClimbSpeed;
			}
			if (climbDown)
			{
				wish.y -= kClimbSpeed;
			}
		}
		else
		{
			_world->setClimbing(_player, false);
			if (!_world->isGrounded(_player))
			{
				wish.x *= kAirControl;
				wish.z *= kAirControl;
			}
		}

		_world->setMove(_player, wish);
		if (_jumpQueued)
		{
			if (_world->isGrounded(_player))
			{
				_world->jump(_player);
			}
			else if (_airJumpsLeft > 0 && !_world->isClimbing(_player))
			{
				--_airJumpsLeft;
				_world->jump(_player);
			}
			_jumpQueued = false;
		}
	}

	void Platformer3DEngineScene::updateOrbit(float delta)
	{
		auto& input = _engine.getInputManager();
		const glm::vec2 mouse = input.getMousePosition();
		if (input.isButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
		{
			if (_orbitDragging)
			{
				_orbitYaw -= (mouse.x - _lastMouse.x) * kOrbitDegreesPerPixel;
				_orbitPitch -= (mouse.y - _lastMouse.y) * kOrbitDegreesPerPixel;
			}
			_orbitDragging = true;
			_lastMouse = mouse;
		}
		else
		{
			_orbitDragging = false;
		}

		if (input.isKeyDown(GLFW_KEY_Q))
		{
			_orbitYaw -= kOrbitYawSpeed * delta;
		}
		if (input.isKeyDown(GLFW_KEY_E))
		{
			_orbitYaw += kOrbitYawSpeed * delta;
		}

		_orbitPitch = glm::clamp(_orbitPitch, kMinPitch, kMaxPitch);
	}

	void Platformer3DEngineScene::syncTransformsFromWorld()
	{
		const hl::physics::Pose playerPose = _world->getPose(_player);

		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, PhysicsLinkComponent>())
		{
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			hl::physics::Pose pose;
			if (link->isCharacter)
			{
				pose = playerPose;
				pose.position.y += link->visualHalfExtents.y;
			}
			else if (link->body.valid())
			{
				pose = _world->getPose(link->body);
			}
			else
			{
				continue;
			}

			const glm::vec3 euler = glm::degrees(glm::eulerAngles(pose.rotation));
			auto transform = entity->GetComponent<hl::TransformComponent>();
			transform->SetPosition(pose.position);
			transform->SetRotation(euler);
			transform->SetScale(glm::vec3(
				2.f * link->visualHalfExtents.x,
				2.f * link->visualHalfExtents.y,
				2.f * link->visualHalfExtents.z));
		}

		const glm::vec3 target{
			playerPose.position.x,
			playerPose.position.y + (kCapsuleHeight + 2.f * kCapsuleRadius) * 0.5f,
			playerPose.position.z
		};
		_worldCamera->setLookAtPose(target, _orbitYaw, _orbitPitch, _orbitDistance);
	}

	bool Platformer3DEngineScene::ladderAabbOverlapsCharacter() const
	{
		if (_world == nullptr || !_player.valid())
		{
			return false;
		}

		const hl::physics::Pose pose = _world->getPose(_player);
		const float charHalfW = kCapsuleRadius;
		const float charHalfH = (kCapsuleHeight + 2.f * kCapsuleRadius) * 0.5f;
		const float charCx = pose.position.x;
		const float charCy = pose.position.y + charHalfH;
		const float charCz = pose.position.z;
		return std::abs(charCx - _ladderPosition.x) < (charHalfW + _ladderHalfExtents.x)
			&& std::abs(charCy - _ladderPosition.y) < (charHalfH + _ladderHalfExtents.y)
			&& std::abs(charCz - _ladderPosition.z) < (charHalfW + _ladderHalfExtents.z);
	}

	bool Platformer3DEngineScene::isCrate(hl::physics::BodyId id) const
	{
		for (const hl::physics::BodyId crate : _crates)
		{
			if (crate == id)
			{
				return true;
			}
		}
		return false;
	}
}
