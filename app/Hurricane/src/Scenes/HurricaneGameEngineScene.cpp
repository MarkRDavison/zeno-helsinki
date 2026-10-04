#include "Scenes/HurricaneGameEngineScene.hpp"
#include <Scenes/SceneHost.hpp>
#include "EntityPushConstantObject.hpp"
#include <Components/EntityComponent.hpp>
#include <Components/CollisionComponent.hpp>
#include <Components/HealthComponent.hpp>
#include <Components/PlayerLoadoutComponent.hpp>
#include <Events/EnemySpawnEvent.hpp>
#include <Events/PlayerLifeLostEvent.hpp>
#include <Events/PlayerScoreEvent.hpp>
#include <GameCamera.hpp>
#include <UiCamera.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <string>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/System/Utils/Xml.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/LogicalResource.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <Systems/PlayerControlSystem.hpp>
#include <Systems/WeaponFiringSystem.hpp>
#include <Systems/ProjectileUpdateSystem.hpp>
#include <Systems/CollisionDetectionSystem.hpp>
#include <Systems/CollisionResolutionSystem.hpp>
#include <Systems/EntityDeathSystem.hpp>
#include <Systems/SpriteClipSystem.hpp>
#include <Systems/EnemySpawnSystem.hpp>
#include <Systems/EnemyUpdateSystem.hpp>
#include <Systems/PickupUpdateSystem.hpp>
#include <Systems/BombSystem.hpp>
#include <GLFW/glfw3.h>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <algorithm>
#include <cmath>
#include <format>
#include <string>
#include <vector>
#include <helsinki/Ui/Widget.hpp>

namespace hur
{

	HurricaneGameEngineScene::HurricaneGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		GameStateService& gameStateService,
		ResourceService& resourceService,
		SceneHost& sceneHost
	) :
		EngineScene(engine),
		_engineConfig(engineConfig),
		_gameStateService(gameStateService),
		_resourceService(resourceService),
		_sceneHost(sceneHost)
	{
        _cameras.insert({ "Ui", new UiCamera() });
        _cameras.insert({ "Game", new GameCamera() });
		_engine.getEventBus().AddListener(this);
	}
	HurricaneGameEngineScene::~HurricaneGameEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
		_sceneHost.onSceneDestroyed();
	}

	void HurricaneGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
        auto uiRenderpassinfo = hl::RenderpassInfo
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
                    .clear = VkClearValue{.color = { 0.0f, 0.0f, 0.0f, 0.0f} }
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
                        .enableBlending = true,
                        .viewport = {
                            .mode = hl::ViewportMode::FixedAspect,
                            .width = HurricaneConstants::Width,
                            .height = HurricaneConstants::Height,
                        }
                    }
                }
            }
        };
        auto sceneRenderpassInfo = hl::RenderpassInfo
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
                                        .resource = "sheet"
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
            .materialSystem = &_engine.getMaterialSystem(),
            .rootPath = _engineConfig.RootPath
        };

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

                const auto uvRect = glm::vec4((float)x, (float)y, (float)(x + w), (float)(y + h)) / TEX_SIZE;
                frameData.push_back({ .uvRect = uvRect });
                _resourceService.addSpriteIndexAndSize(name, idx, glm::vec2((float)w, (float)h), uvRect);

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

        resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
            "roboto",
            resourceContext);

        _uiSheetDefinition = hl::ResourceDefinition
        {
            .name = "ui_sheet",
            .type = "logical",
            .resources =
            {
                hl::ResourceDefinition::Child
                {
                    .name = "white",
                    .type = "texture"
                },
                hl::ResourceDefinition::Child
                {
                    .name = "roboto",
                    .type = "texture"
                },
                hl::ResourceDefinition::Child
                {
                    .name = "sheet",
                    .type = "texture"
                }
            }
        };

        _uiSheetHandle = resourceManager.LoadLogical(_uiSheetDefinition, [&](const hl::ResourceDefinition::Child& child)
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

        registerPipelineDraw(
            "sprite_pipeline",
            [&](hl::PipelineDrawData& pdd) -> void
            {
                for (const auto& entity : pdd.scene->getEntities())
                {
                    if (!entity->HasComponents<hl::TransformComponent, hl::SpriteComponent, EntityComponent>())
                    {
                        continue;
                    }

                    const auto& transform = entity->GetComponent<hl::TransformComponent>();
                    const auto& sprite = entity->GetComponent<hl::SpriteComponent>();
                    const auto& ec = entity->GetComponent<EntityComponent>();

                    auto modelTransform = transform->GetTransformMatrix();

                    const auto size = ec->Size;

                    auto pc = hl::SpritePushConstantObject
                    {
                        .model = modelTransform,
                        .size = size,
                        .offset = glm::vec2(-size.x * 0.5f, -size.y * 0.5f),
                        .frameIndex = sprite->getFrameDataIndex(),
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
                _uiBatch.draw(pdd);
            });

        setGameState(GameState::INIT);

        _scene.addSystem(new PlayerControlSystem(
            _engine.getInputManager(),
            _engine.getEventBus(),
            this->_scene,
            _resourceService));

        _scene.addSystem(new WeaponFiringSystem(
            _engine.getEventBus(),
            this->_scene,
            _resourceService));

        _scene.addSystem(new ProjectileUpdateSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new EnemyUpdateSystem(
            _engine.getEventBus(),
            this->_scene));

        _scene.addSystem(new PickupUpdateSystem(
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
            this->_scene,
            _resourceService));

        _scene.addSystem(new BombSystem(
            _engine.getEventBus(),
            this->_scene,
            _resourceService));

        _scene.addSystem(new SpriteClipSystem(
            this->_scene));

        _scene.addSystem(new EnemySpawnSystem(
            _engine.getEventBus(),
            this->_scene,
            _resourceService));

		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);

        _uiBatch.initialise(device);

        buildHud(resourceManager.GetResource<hl::FontResource>("roboto"));
	}

	void HurricaneGameEngineScene::update(uint32_t currentFrame, float delta)
	{
        _uiBatch.begin();

        if (_state == GameState::INIT)
        {
            transitionFromInitToPlaying();
            return;
        }

        if (_state == GameState::PLAYING && _respawnTimer <= 0.0f)
        {
            _scene.update(delta);
        }

        applyPendingDeath();

        if (_state == GameState::PLAYING && _respawnTimer > 0.0f)
        {
            _respawnTimer -= delta;
            if (_respawnTimer <= 0.0f)
            {
                _respawnTimer = 0.0f;
                spawnPlayer();
            }
        }

        updateUi();
	}

    void HurricaneGameEngineScene::updateGpuResources(uint32_t currentFrame)
    {
        _uiBatch.updateGpuResources(currentFrame);
    }

    void HurricaneGameEngineScene::additionalCleanup()
    {
        _uiBatch.destroy();
    }

    void HurricaneGameEngineScene::spawnPlayer()
    {
        auto existing = _scene.getEntity("Player");

        if (existing != nullptr)
        {
            return;
        }

        if (_gameStateService.getLivesRemaining() <= 0)
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
        entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(
            static_cast<int>(_resourceService.getIndex(sc->SpriteName)));
        entity->AddComponent< HealthComponent>(10, 10);
        entity->AddComponent<PlayerLoadoutComponent>();
        auto* weapon = entity->AddComponent<WeaponComponent>();
        applyWeapon(*weapon, WeaponTypeSingleLaser);
        auto cc = entity->AddComponent<CollisionComponent>();
        cc->layer = CollisionLayer::Player;
        cc->mask = CollisionLayer::EnemyBullet | CollisionLayer::Enemy | CollisionLayer::Pickup;
    }

    void HurricaneGameEngineScene::buildHud(hl::FontResource* font)
    {
        _typeface = std::make_unique<FontTypeface>(font);

        _layoutRoot = std::make_unique<hl::ui::Node>();
        _layoutRoot->setFillParent();

        _lives = std::make_unique<hl::ui::IconRow>(_layoutRoot->addChild());
        _lives->iconSize = _resourceService.getSize("playerLife1_blue");
        _lives->gap = 8.0f;
        _lives->uvRect = _resourceService.getUvRect("playerLife1_blue");
        _lives->color = { 1.0f, 1.0f, 1.0f };

        _bombs = std::make_unique<hl::ui::IconRow>(_layoutRoot->addChild());
        _bombs->iconSize = _resourceService.getSize("star3");
        _bombs->gap = 8.0f;
        _bombs->uvRect = _resourceService.getUvRect("star3");
        _bombs->color = { 1.0f, 1.0f, 1.0f };

        _score = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
        _score->color = { 1.0f, 1.0f, 1.0f };
        _score->setText("Score: 0", 24);

        _status = std::make_unique<hl::ui::Label>(_layoutRoot->addChild(), *_typeface);
        _status->color = { 1.0f, 1.0f, 1.0f };
        _status->setText("", 24);

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

        _overlayScore = std::make_unique<hl::ui::Label>(column.addChild(), *_typeface);
        _overlayScore->color = { 1.0f, 1.0f, 1.0f };

        _resume = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
        _resume->setText("Resume", 48);
        _resume->onClick = [this]()
        {
            setGameState(GameState::PLAYING);
        };

        _titleButton = std::make_unique<hl::ui::Button>(column.addChild(), *_typeface);
        _titleButton->setText("Title", 48);
        _titleButton->onClick = [this]()
        {
            goToTitle();
        };
    }

    namespace
    {
        void pinHudCorner(hl::ui::Node& node, bool topRight, glm::vec2 inset)
        {
            const glm::vec2 size = node.intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
            if (topRight)
            {
                node.setTopRight(size);
                node.relative = { -inset.x, inset.y };
            }
            else
            {
                node.setTopLeft(size);
                node.relative = inset;
            }
            node.intrinsicSize.reset();
        }
    }

    void HurricaneGameEngineScene::updateUi()
    {
        _score->setText("Score: " + std::to_string(_gameStateService.getScore()), 24);
        _lives->setCount(std::max(0, _gameStateService.getLivesRemaining()));

        int bombCount = 0;
        if (auto* player = _scene.getEntity("Player"))
        {
            if (auto* loadout = player->GetComponent<PlayerLoadoutComponent>())
            {
                bombCount = std::max(0, loadout->bombs);
            }
        }
        _bombs->setCount(bombCount);

        if (_respawnTimer > 0.0f)
        {
            _status->setText(std::format("Respawn in {:.0f}", std::ceil(_respawnTimer)), 24);
        }
        else
        {
            _status->setText("", 24);
        }

        hl::ui::prepareTree(*_layoutRoot);
        pinHudCorner(_score->node(), true, { 16.0f, 16.0f });
        pinHudCorner(_lives->node(), false, { 16.0f, 16.0f });
        pinHudCorner(_bombs->node(), false, { 16.0f, 16.0f + _lives->iconSize.y + 8.0f });

        const glm::vec2 statusSize = _status->node().intrinsicSize.value_or(glm::vec2{ 0.0f, 0.0f });
        _status->node().setTopCenter(statusSize);
        _status->node().relative = { 0.0f, 16.0f };
        _status->node().intrinsicSize.reset();

        const hl::ui::Box playfield{
            0.0f,
            0.0f,
            static_cast<float>(HurricaneConstants::Width),
            static_cast<float>(HurricaneConstants::Height) };

        hl::ui::layout(*_layoutRoot, playfield);

        UiBatchPaint paint(_uiBatch);

        if (overlayVisible())
        {
            refreshOverlay();
            hl::ui::prepareTree(*_overlayRoot);
            hl::ui::layout(*_overlayRoot, playfield);
            hl::ui::dispatch(
                *_overlayRoot,
                readPlayfieldPointer(
                    _engine,
                    static_cast<float>(HurricaneConstants::Width),
                    static_cast<float>(HurricaneConstants::Height)));
            hl::ui::paintTree(*_overlayRoot, paint);
        }

        hl::ui::paintTree(*_layoutRoot, paint);
    }

    void HurricaneGameEngineScene::transitionFromInitToPlaying()
    {
        _gameStateService.setLivesRemaining(3);
        _gameStateService.setScore(0);
        _pendingDeath = false;
        _respawnTimer = 0.0f;

        spawnPlayer();
        _engine.getEventBus().PublishEvent(EnemySpawnEvent());

        setGameState(GameState::PLAYING);
    }

	void HurricaneGameEngineScene::OnEvent(const hl::Event& event)
	{
        if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_ESCAPE)
            {
                if (_state == GameState::PLAYING)
                {
                    setGameState(GameState::PAUSED);
                }
                else if (_state == GameState::PAUSED)
                {
                    setGameState(GameState::PLAYING);
                }
            }
        }
        else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
        {
            handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
        }
        else if (dynamic_cast<const PlayerLifeLostEvent*>(&event) != nullptr)
        {
            _pendingDeath = true;
        }
        else if (auto pse = dynamic_cast<const PlayerScoreEvent*>(&event))
        {
            _gameStateService.incrementScore(pse->getAmount());
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

    void HurricaneGameEngineScene::applyPendingDeath()
    {
        if (!_pendingDeath)
        {
            return;
        }

        _pendingDeath = false;
        clearHostiles();

        const int lives = _gameStateService.getLivesRemaining() - 1;
        _gameStateService.setLivesRemaining(std::max(0, lives));
        _respawnTimer = 0.0f;

        if (lives <= 0)
        {
            setGameState(GameState::GAME_OVER);
            return;
        }

        _respawnTimer = 2.5f;
    }

    void HurricaneGameEngineScene::clearHostiles()
    {
        std::vector<int> ids;
        for (const char* tag : { "ENEMY", "PROJECTILE", "PICKUP", "PLAYER_FX" })
        {
            for (auto* entity : _scene.getEntitiesByTag(tag))
            {
                ids.push_back(entity->Id);
            }
        }

        for (const int id : ids)
        {
            _scene.removeEntity(id);
        }
    }

    void HurricaneGameEngineScene::goToTitle()
    {
        _sceneHost.goTitle();
    }

    bool HurricaneGameEngineScene::overlayVisible() const
    {
        return _state == GameState::PAUSED || _state == GameState::GAME_OVER;
    }

    void HurricaneGameEngineScene::refreshOverlay()
    {
        if (_state == GameState::PAUSED)
        {
            _overlayHeading->setText("Paused", 48);
            _overlayScore->setText("", 24);
            _resume->setText("Resume", 48);
            _resume->hitTestEnabled = true;
        }
        else
        {
            _overlayHeading->setText("Game Over", 48);
            _overlayScore->setText("Score: " + std::to_string(_gameStateService.getScore()), 24);
            _resume->setText("", 24);
            _resume->hitTestEnabled = false;
        }

        _titleButton->hitTestEnabled = true;
    }
}