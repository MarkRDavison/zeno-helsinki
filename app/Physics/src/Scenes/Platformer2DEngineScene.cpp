#include <Scenes/Platformer2DEngineScene.hpp>
#include <Components/PhysicsLinkComponent.hpp>
#include <EntityPushConstantObject.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <Scenes/TitleEngineScene.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
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
		// 1 physics unit = 1 meter. Screen pixels are presentation only.
		constexpr float kPixelsPerMeter = 48.f;
		constexpr float kWalkSpeed = 6.5f;
		constexpr float kClimbSpeed = 5.5f;
		constexpr float kAirControl = 0.7f;
		constexpr float kCapsuleRadius = 0.4f;
		constexpr float kCapsuleHeight = 1.2f;
	}

	Platformer2DEngineScene::Platformer2DEngineScene(
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
		_engine.getEventBus().AddListener(this);

		hl::physics::WorldSettings settings;
		settings.dim = hl::physics::Dim::D2;
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

	Platformer2DEngineScene::~Platformer2DEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void Platformer2DEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		auto sceneRenderpassInfo = hl::RenderpassInfo
		{
			.name = "scene_pass",
			.inputs = {},
			.outputs =
			{
				hl::ResourceInfo
				{
					.name = "scene_color",
					.type = hl::ResourceType::Color,
					.format = "VK_FORMAT_B8G8R8A8_SRGB"
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
						.name = "entity_pipeline",
						.shaderVert = _engineConfig.RootPath + std::string("/data/shaders/entity.vert"),
						.shaderFrag = std::string(hl::RendererShaderRoot) + "/entity.frag",
						.descriptorSets =
						{
							hl::DescriptorSetInfo
							{
								.name = "",
								.bindings =
								{
									hl::DescriptorBinding
									{
										.binding = 0,
										.type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
										.stage = "VERTEX",
										.resource = cameraMatrixResourceId,
										.count = MAX_CAMERAS
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
						.pushConstantSize = sizeof(EntityPushConstantObject)
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
				{ .pos = { -0.5f, 0.5f } },
			});

		buildCourse();
		addTextEntity(_scene, _engine, "hint", "A/D walk  Space jump (double)  Up/Down climb  Escape: title", 36);
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
			"entity_pipeline",
			[&](hl::PipelineDrawData& pdd) -> void
			{
				const auto& resource = _resourceManager->GetResource<hl::VertexArrayResource>("unit_quad");
				for (const auto& entity : pdd.scene->getEntities())
				{
					if (!entity->HasComponents<hl::TransformComponent, PhysicsLinkComponent>())
					{
						continue;
					}

					auto transform = entity->GetComponent<hl::TransformComponent>();
					const auto& link = entity->GetComponent<PhysicsLinkComponent>();
					const auto& modelTransform = transform->GetTransformMatrix();
					auto pc = EntityPushConstantObject
					{
						.model = modelTransform,
						.color = link->color
					};

					vkCmdPushConstants(
						pdd.commandBuffer,
						pdd.pipeline->getPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(EntityPushConstantObject),
						&pc);
					VkBuffer vertexBuffers[] = { resource->_vertexBuffer._buffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(pdd.commandBuffer, 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(
						pdd.commandBuffer,
						resource->_indexBuffer._buffer,
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
					vkCmdDrawIndexed(pdd.commandBuffer, resource->getIndexCount(), 1, 0, 0, 0);
				}
			});
	}

	void Platformer2DEngineScene::update(uint32_t, float delta)
	{
		if (_world == nullptr || !_player.valid())
		{
			return;
		}

		applyInput();
		_ladderContactThisStep = false;
		_world->step(delta);
		if (_world->isGrounded(_player))
		{
			_airJumpsLeft = 1;
		}
		_ladderOverlap = _ladderContactThisStep || ladderAabbOverlapsCharacter();
		syncTransformsFromWorld();
	}

	void Platformer2DEngineScene::additionalCleanup()
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

	void Platformer2DEngineScene::OnEvent(const hl::Event& event)
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

	void Platformer2DEngineScene::handleWindowSizeChange(int width, int height)
	{
		_viewportWidth = width;
		_viewportHeight = height;
		centerTextAt(_scene, _engine, width, height, "hint", static_cast<float>(-height) / 3.0f + 40.0f);
	}

	hl::Entity* Platformer2DEngineScene::addVisual(
		const std::string& name,
		const glm::vec3& visualHalfExtents,
		const glm::vec4& color,
		bool isCharacter)
	{
		auto entity = _scene.addEntity(name);
		entity->AddTag("PHYSICS");
		entity->AddComponent<hl::TransformComponent>();
		auto link = entity->AddComponent<PhysicsLinkComponent>();
		link->visualHalfExtents = visualHalfExtents;
		link->color = color;
		link->isCharacter = isCharacter;
		return entity;
	}

	void Platformer2DEngineScene::buildCourse()
	{
		// Course is authored in meters (XY). Z is not a gameplay axis.
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
				throw std::runtime_error("Platformer2DEngineScene: failed to create static body");
			}
			return link->body;
		};

		addStatic(
			"floor",
			{ 12.f, 0.5f, 0.05f },
			{ 10.f, -0.5f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.35f, 0.38f, 1.f));

		const glm::quat gentle = glm::angleAxis(glm::radians(25.f), glm::vec3(0.f, 0.f, 1.f));
		addStatic(
			"ramp",
			{ 4.f, 0.2f, 0.05f },
			{ 5.f, 1.9f, 0.f },
			gentle,
			glm::vec4(0.55f, 0.75f, 0.95f, 1.f));

		addStatic(
			"platform_low",
			{ 1.5f, 0.2f, 0.05f },
			{ 8.5f, 1.5f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.85f, 0.9f, 1.f),
			true);
		addStatic(
			"platform_mid",
			{ 1.5f, 0.2f, 0.05f },
			{ 9.f, 2.8f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.35f, 0.85f, 0.9f, 1.f),
			true);

		addStatic(
			"platform_exit",
			{ 2.f, 0.2f, 0.05f },
			{ 14.2f, 4.2f, 0.f },
			glm::quat(1.f, 0.f, 0.f, 0.f),
			glm::vec4(0.7f, 0.7f, 0.72f, 1.f));

		const glm::quat steep = glm::angleAxis(glm::radians(70.f), glm::vec3(0.f, 0.f, 1.f));
		addStatic(
			"steep",
			{ 2.f, 0.2f, 0.05f },
			{ 18.f, 1.2f, 0.f },
			steep,
			glm::vec4(0.85f, 0.3f, 0.25f, 1.f));

		{
			_ladderPosition = { 12.f, 2.f, 0.f };
			_ladderHalfExtents = { 0.25f, 2.f, 0.05f };
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
				throw std::runtime_error("Platformer2DEngineScene: failed to create ladder sensor");
			}
		}

		{
			auto* entity = addVisual("crate", { 0.25f, 0.25f, 0.05f }, glm::vec4(0.72f, 0.5f, 0.22f, 1.f), false);
			hl::physics::BodyDesc desc;
			desc.shape.kind = hl::physics::Shape::Kind::Box;
			desc.shape.halfExtents = { 0.25f, 0.25f, 0.05f };
			desc.motion = hl::physics::MotionType::Dynamic;
			desc.layer = hl::physics::Layer::Moving;
			desc.pose.position = { 4.f, 1.f, 0.f };
			desc.mass = 2.f;
			desc.userData = static_cast<std::uint64_t>(entity->Id);
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->body = _world->createBody(desc);
			if (!link->body.valid())
			{
				throw std::runtime_error("Platformer2DEngineScene: failed to create crate");
			}
			_crates.push_back(link->body);
		}

		{
			const glm::vec3 playerHalf{
				kCapsuleRadius,
				(kCapsuleHeight + 2.f * kCapsuleRadius) * 0.5f,
				0.05f
			};
			auto* entity = addVisual("player", playerHalf, glm::vec4(0.95f, 0.85f, 0.2f, 1.f), true);
			hl::physics::CharacterDesc desc;
			desc.dim = hl::physics::Dim::D2;
			desc.pose.position = { 1.f, 2.f, 0.f };
			desc.capsuleRadius = kCapsuleRadius;
			desc.capsuleHeight = kCapsuleHeight;
			desc.maxSlopeAngleDeg = 45.f;
			desc.jumpSpeed = 11.f;
			_player = _world->createCharacter(desc);
			if (!_player.valid())
			{
				throw std::runtime_error("Platformer2DEngineScene: failed to create character");
			}
			auto link = entity->GetComponent<PhysicsLinkComponent>();
			link->character = _player;
		}
	}

	void Platformer2DEngineScene::applyInput()
	{
		auto& input = _engine.getInputManager();
		glm::vec3 wish{ 0.f };
		if (input.isKeyDown(GLFW_KEY_A) || input.isKeyDown(GLFW_KEY_LEFT))
		{
			wish.x -= kWalkSpeed;
		}
		if (input.isKeyDown(GLFW_KEY_D) || input.isKeyDown(GLFW_KEY_RIGHT))
		{
			wish.x += kWalkSpeed;
		}

		const bool climbUp = input.isKeyDown(GLFW_KEY_W) || input.isKeyDown(GLFW_KEY_UP);
		const bool climbDown = input.isKeyDown(GLFW_KEY_S) || input.isKeyDown(GLFW_KEY_DOWN);
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

	void Platformer2DEngineScene::syncTransformsFromWorld()
	{
		const hl::physics::Pose playerPose = _world->getPose(_player);
		const glm::vec2 focus(playerPose.position.x, playerPose.position.y);
		const glm::vec2 screenCenter(
			static_cast<float>(_viewportWidth) * 0.5f,
			static_cast<float>(_viewportHeight) * 0.5f);

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

			pose.position.z = 0.f;
			const glm::vec2 world(pose.position.x, pose.position.y);
			// Physics is Y-up; Camera2D / Pong pixel space has y = 0 at the top.
			const glm::vec2 screen{
				(world.x - focus.x) * kPixelsPerMeter + screenCenter.x,
				(focus.y - world.y) * kPixelsPerMeter + screenCenter.y
			};
			const glm::vec3 euler = glm::degrees(glm::eulerAngles(pose.rotation));

			auto transform = entity->GetComponent<hl::TransformComponent>();
			transform->SetPosition(glm::vec3(screen.x, screen.y, 0.f));
			transform->SetRotation(glm::vec3(0.f, 0.f, -euler.z));
			transform->SetScale(glm::vec3(
				kPixelsPerMeter * 2.f * link->visualHalfExtents.x,
				kPixelsPerMeter * 2.f * link->visualHalfExtents.y,
				1.f));
		}
	}

	bool Platformer2DEngineScene::ladderAabbOverlapsCharacter() const
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
		return std::abs(charCx - _ladderPosition.x) < (charHalfW + _ladderHalfExtents.x)
			&& std::abs(charCy - _ladderPosition.y) < (charHalfH + _ladderHalfExtents.y);
	}

	bool Platformer2DEngineScene::isCrate(hl::physics::BodyId id) const
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
