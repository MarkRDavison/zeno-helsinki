#include "Scenes/HurricaneGameEngineScene.hpp"
#include "EntityPushConstantObject.hpp"
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Events/EnemySpawnEvent.hpp>
#include <Events/PlayerLifeLostEvent.hpp>
#include <Events/PlayerScoreEvent.hpp>
#include <GameCamera.hpp>
#include <UiCamera.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/Utils/Xml.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <Systems/PlayerControlSystem.hpp>
#include <Systems/WeaponFiringSystem.hpp>
#include <Systems/ProjectileUpdateSystem.hpp>
#include <Systems/CollisionDetectionSystem.hpp>
#include <Systems/CollisionResolutionSystem.hpp>
#include <Systems/EntityDeathSystem.hpp>
#include <Systems/EnemySpawnSystem.hpp>
#include <Systems/EnemyUpdateSystem.hpp>
#include <GLFW/glfw3.h>
#include <Ui/UiLayout.hpp>

constexpr auto MAX_UI_VERTEXES = 1024;

namespace hur
{

	HurricaneGameEngineScene::HurricaneGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig
	) :
		EngineScene(engine),
		_engineConfig(engineConfig),
        _uiRoot(engine.getInputManager())
	{
        _cameras.insert({ "Ui", new UiCamera() });
        _cameras.insert({ "Game", new GameCamera() });
		_engine.getEventBus().AddListener(this);
        _engine.getEventBus().AddListener(&_uiRoot);
	}
	HurricaneGameEngineScene::~HurricaneGameEngineScene()
	{
        _engine.getEventBus().RemoveListener(&_uiRoot);
		_engine.getEventBus().RemoveListener(this);
	}

	void HurricaneGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager,
		hl::MaterialSystem& materialSystem)
	{
        auto uiRenderpassinfo = hl::RenderpassInfo
        {
            .name = "ui_renderpass",
            .useMultiSampling = false,
            .inputs = {},
            .outputs =
            {
                hl::ResourceInfo
                {
                    .name = "ui_color",
                    .type = hl::ResourceType::Color,
                    .format = "VK_FORMAT_B8G8R8A8_SRGB",
                    .clear = VkClearValue{.color = { 0.0f, 0.0f, 0.0f, 0.0f} }
                }
            },
            .pipelineGroups =
            {
                {
                    hl::PipelineInfo
                    {
                        .name = "ui_pipeline",
                        .shaderVert = _engineConfig.RootPath + "/data/shaders/ui.vert",
                        .shaderFrag = _engineConfig.RootPath + "/data/shaders/ui.frag",
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
                                    .offset = offsetof(hl::VertexUi, pos)
                                },
                                {
                                    .name = "inColor",
                                    .format = hl::VertexAttributeFormat::Vec3,
                                    .location = 1,
                                    .offset = offsetof(hl::VertexUi, color)
                                }
                            },
                            .stride = sizeof(hl::VertexUi)
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
                        .enableBlending = false,
                    }
                }
            }
        };
        auto sceneRenderpassInfo = hl::RenderpassInfo
        {
            .name = "sprite_pass",
            .useMultiSampling = false,
            .inputs = {},
            .outputs =
            {
                hl::ResourceInfo
                {
                    .name = "scene_color",
                    .type = hl::ResourceType::Color,
                    .format = "VK_FORMAT_B8G8R8A8_SRGB",
                    .clear = VkClearValue{.color = { 0.0f, 0.2f, 0.8f, 1.0f}}
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
                        .shaderFrag = _engineConfig.RootPath + "/data/shaders/sprites.frag",
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
                                        .resource = "sheet"
                                    }
                                }
                            }
                        },
                        .rasterState =
                        {
                            .cullMode = VK_CULL_MODE_NONE
                        },
                        .enableBlending = true,
                        .pushConstantSize = sizeof(hl::SpritePushConstantObject),
                        .viewport = {
                            .mode = hl::ViewportMode::FixedAspect,
                            .width = HurricaneConstants::Width,
                            .height = HurricaneConstants::Height,
                        }
                    }
                }
            }
        };



        std::vector<hl::RenderpassInfo> renderpasses =
        {
            uiRenderpassinfo,
            sceneRenderpassInfo,
            // TODO: ref vars for the outputs???
            hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "scene_color", "ui_color" })
        };

        hl::ResourceContext resourceContext
        {
            .device = &device,
            .pool = &transferCommandPool,
            .resourceManager = &resourceManager,
            .materialSystem = &materialSystem,
            .rootPath = _engineConfig.RootPath
        };

        // TODO: Move to base and generate texture programatically
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            hl::MaterialSystem::FallbackTextureName,
            resourceContext);


        {

            const auto& doc = hl::Xml::parseFromFile(_engineConfig.RootPath + "/data/spritesheet/sheet.xml");

            std::vector<hl::FrameDataStorageBufferObject> frameData;

            const auto& subTextures = doc.selectMany("TextureAtlas/SubTexture");

            const constexpr float TEX_SIZE = 1024.0f;
            std::size_t idx = 0;
            for (const auto& subTexture : subTextures)
            {
                const auto name = hl::String::ReplaceAll(subTexture->attributes["name"], ".png", "");
                const auto x = std::stoi(subTexture->attributes["x"]);
                const auto y = std::stoi(subTexture->attributes["y"]);
                const auto w = std::stoi(subTexture->attributes["width"]);
                const auto h = std::stoi(subTexture->attributes["height"]);

                frameData.push_back({ .uvRect = glm::vec4((float)x, (float)y, (float)(x + w), (float)(y + h)) / TEX_SIZE });
                _resourceService.addSpriteIndexAndSize(name, idx, glm::vec2((float)w, (float)h));

                idx++;
            }

            _spriteSheetSSBOResourceHandle = resourceManager.Load<hl::StorageBufferResource>(
                "spritesheet_frame_ssbo",
                resourceContext,
                sizeof(hl::FrameDataStorageBufferObject),
                512);

            auto ssbo = _spriteSheetSSBOResourceHandle.Get();

            for (uint32_t i = 0; i < (uint32_t)frameData.size(); ++i)
            {
                ssbo->writeToBuffer(&frameData[i], i);
            }
        }

        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "sheet",
            resourceContext);

        EngineScene::initialise(
            cameraMatrixResourceId,
            device,
            swapChain,
            graphicsCommandPool,
            transferCommandPool,
            resourceManager,
            materialSystem,
            renderpasses);

        registerPipelineDraw(
            "sprite_pipeline",
            [&](hl::PipelineDrawData& pdd) -> void
            {
                for (const auto& entity : pdd.scene->getEntities())
                {
                    if (!entity->HasComponents<hl::TransformComponent, hl::SpriteComponent>())
                    {
                        continue;
                    }

                    const auto& transform = entity->GetComponent<hl::TransformComponent>();
                    const auto& sprite = entity->GetComponent<hl::SpriteComponent>();
                    const auto& ec = entity->GetComponent<EntityComponent>();

                    auto modelTransform = transform->GetTransformMatrix();

                    const auto size = _resourceService.getSize(ec->SpriteName);

                    auto pc = hl::SpritePushConstantObject
                    {
                        .model = modelTransform,
                        .size = size,
                        .offset = glm::vec2(-size.x * 0.5f, -size.y * 0.5f),
                        .frameIndex = (int)_resourceService.getIndex(ec->SpriteName),
                        .cameraIndex = (int)getCameraIndex("Game")
                    };

                    vkCmdPushConstants(
                        pdd.commandBuffer,
                        pdd.pipeline->getPipelineLayout(),
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(hl::SpritePushConstantObject),
                        &pc
                    );

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


        registerPipelineDraw(
            "ui_pipeline", 
            [&](hl::PipelineDrawData& pdd) -> void 
            {
                _uiRoot.draw(pdd);
            });

        setGameState(GameState::INIT);

        _scene.addSystem(new PlayerControlSystem(
            _engine.getInputManager(),
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new WeaponFiringSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new ProjectileUpdateSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new EnemyUpdateSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new CollisionDetectionSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new CollisionResolutionSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new EntityDeathSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new EnemySpawnSystem(
            _engine.getEventBus(),
            this->_scene,
            _resourceService));

		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);

        _uiRoot.initialise(device);

        _elements.push_back(UiElement
            {
                .size = {256.0f, 48.0f},
                .offset = {16.0f, 16.0f},
                .anchor = UiAnchor::TopLeft
            });

        _elements.push_back(UiElement
            {
                .size = {128.0f, 48.0f},
                .offset = {16.0f, 16.0f},
                .anchor = UiAnchor::TopRight
            });

        _elements.push_back(UiElement
            {
                .size = {384.0f, 32.0f},
                .offset = {16.0f, 16.0f},
                .anchor = UiAnchor::BottomLeft
            });
	}

	void HurricaneGameEngineScene::update(uint32_t currentFrame, float delta)
	{
        _uiRoot.update(delta);

        if (_state == GameState::INIT)
        {
            transitionFromInitToPlaying();
        }
        else if (_state == GameState::PLAYING)
        {
            _scene.update(delta);
            updateUi();
        }
	}

    void HurricaneGameEngineScene::updateGpuResources(uint32_t currentFrame)
    {
        _uiRoot.updateGpuResources(currentFrame);
    }

    void HurricaneGameEngineScene::additionalCleanup()
    {
        _uiRoot.destroy();
    }

    void HurricaneGameEngineScene::spawnPlayer()
    {
        auto existing = _scene.getEntity("Player");

        if (existing != nullptr)
        {
            return;
        }

        if (_gameStateService.getLivesRemaining() < 0)
        {
            return;
        }

        auto entity = _scene.addEntity("Player");
        entity->AddTag("SPRITE");
        entity->AddTag("ENTITY");
        entity->AddTag("COLLIDER");
        entity->AddTag("PLAYER");
        auto sc = entity->AddComponent<EntityComponent>();
        sc->SpriteName = "playerShip1_blue";
        sc->Size = _resourceService.getSize(sc->SpriteName);
        entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(
            HurricaneConstants::Width / 2.0f,
            HurricaneConstants::Height - sc->Size.y / 2.0f,
            0.0f));
        entity->AddComponent<hl::SpriteComponent>();
        entity->AddComponent< HealthComponent>(10, 10);
        auto cc = entity->AddComponent<CollisionComponent>();
        cc->layer = CollisionLayer::Player;
        cc->mask = CollisionLayer::EnemyBullet | CollisionLayer::Enemy;
    }

    void HurricaneGameEngineScene::transitionFromGameOverToInit()
    {
        if (_state != GameState::GAME_OVER)
        {
            return;
        }

        // TODO: MOVE _state to game state service???

        for (const auto& e : _scene.getEntities())
        {
            _scene.removeEntity(e->Id);
        }

        // TODO: BETTER RESET METHOD?
        _scene.update();

        setGameState(GameState::INIT);
    }

    void HurricaneGameEngineScene::updateUi()
    {
        for (auto& e : _elements)
        {
            e.calculatedRect = UiLayout::Calculate(e, { _width, _height });
        }

        for (const auto& e : _elements)
        {
            _uiRoot.addQuad(e.calculatedRect, { 1.0f, 0.0f, 0.0f, 0.0f });
        }
    }

    void HurricaneGameEngineScene::transitionFromInitToPlaying()
    {
        spawnPlayer();
        _engine.getEventBus().PublishEvent(EnemySpawnEvent());

        _gameStateService.setLivesRemaining(3);
        _gameStateService.setScore(0);

        setGameState(GameState::PLAYING);
    }

	void HurricaneGameEngineScene::OnEvent(const hl::Event& event)
	{
        if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_ENTER)
            {
                _engine.getEventBus().PublishEvent(EnemySpawnEvent());
            }
            else if (code == GLFW_KEY_R)
            {
                spawnPlayer();
            }
            else if (code == GLFW_KEY_B)
            {
                transitionFromGameOverToInit();
            }
        }
        else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
        {
            handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
        }
        else if (auto plle = dynamic_cast<const PlayerLifeLostEvent*>(&event))
        {
            const auto lives = _gameStateService.getLivesRemaining() - 1;

            if (lives < 0)
            {
                setGameState(GameState::GAME_OVER);
            }
            else
            {
                _gameStateService.setLivesRemaining(lives);
                // TODO: 
                // Clear out enemies, restart wave/enemy spawning?
                // Spawn player again
                std::cout << "LIVES: " << _gameStateService.getLivesRemaining() << std::endl;
            }
        }
        else if (auto pse = dynamic_cast<const PlayerScoreEvent*>(&event))
        {
            _gameStateService.incrementScore(pse->getAmount());

            std::cout << "SCORE: " << _gameStateService.getScore() << std::endl;
        }
	}

	void HurricaneGameEngineScene::handleWindowSizeChange(int width, int height)
	{
        _width = width;
        _height = height;
	}

    void HurricaneGameEngineScene::setGameState(GameState state)
    {
        _state = state;
        
        handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
    }
}