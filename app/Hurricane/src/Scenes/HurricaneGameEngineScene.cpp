#include "Scenes/HurricaneGameEngineScene.hpp"
#include "EntityPushConstantObject.hpp"
#include <Components/EntityComponent.hpp>
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
#include <GLFW/glfw3.h>


namespace hur
{

	HurricaneGameEngineScene::HurricaneGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig
	) :
		EngineScene(engine),
		_engineConfig(engineConfig)
	{
        _cameras.insert({ "Ui", new UiCamera() });
        _cameras.insert({ "Game", new GameCamera() });
		_engine.getEventBus().AddListener(this);
	}
	HurricaneGameEngineScene::~HurricaneGameEngineScene()
	{
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
                        .pushConstantSize = sizeof(hl::SpritePushConstantObject)
                    }
                }
            }
        };

        std::vector<hl::RenderpassInfo> renderpasses =
        {
            sceneRenderpassInfo,
            hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId),
            // TODO: ref vars for the outputs???
            hl::RenderGraphHelpers::createCompositeRenderpassInfo({ "scene_color", "text_color" })
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
                _spriteToIndexAndSize.insert({ name, {idx, glm::vec2((float)w, (float)h)}});

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

        {
            auto entity = _scene.addEntity("game_state");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "Start",
                "roboto",
                32);
            entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 0.2f));
        }

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

                    const auto& indexSizeData = _spriteToIndexAndSize[ec->SpriteName];

                    auto pc = hl::SpritePushConstantObject
                    {
                        .model = modelTransform,
                        .size = indexSizeData.second,
                        .offset = glm::vec2(-indexSizeData.second.x * 0.5f, -indexSizeData.second.y * 0.5f),
                        .frameIndex = (int)indexSizeData.first,
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

        setGameState(GameState::INIT);

        _scene.addSystem(new PlayerControlSystem(
            _engine.getInputManager(),
            _engine.getEventBus(),
            this->_scene));

		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
	}

	void HurricaneGameEngineScene::update(uint32_t currentFrame, float delta)
	{
        if (_state == GameState::INIT)
        {
            transitionFromInitToPlaying();
        }
        else if (_state == GameState::PLAYING)
        {
            _scene.update(delta);
        }
	}

    void HurricaneGameEngineScene::transitionFromInitToPlaying()
    {
        auto entity = _scene.addEntity("Player");
        entity->AddTag("SPRITE");
        entity->AddTag("ENTITY");
        entity->AddTag("PLAYER");
        entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(128.0f, 128.0f, 0.0f));
        entity->AddComponent<hl::SpriteComponent>();
        entity->AddComponent<EntityComponent>()->SpriteName = "playerShip1_blue";

        setGameState(GameState::PLAYING);
    }

	void HurricaneGameEngineScene::OnEvent(const hl::Event& event)
	{
        if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_ENTER)
            {
                std::cout << "ENTER PRESSED!" << std::endl;
            }
        }
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void HurricaneGameEngineScene::handleWindowSizeChange(int width, int height)
	{

	}

    void HurricaneGameEngineScene::setGameState(GameState state)
    {
        auto entity = _scene.getEntity("game_state");

        auto tc = entity->GetComponent<hl::TextComponent>();

        if (state == GameState::INIT)
        {
            tc->setString(
                _engine.getTextSystem(), 
                "INIT",
                "roboto",
                32);
        }
        else if (state == GameState::PLAYING)
        {
            tc->setString(
                _engine.getTextSystem(),
                "PLAYING",
                "roboto",
                32);
        }
        else if (state == GameState::GAME_OVER)
        {
            tc->setString(
                _engine.getTextSystem(),
                "GAME_OVER",
                "roboto",
                32);
        }
        else
        {
            tc->setString(
                _engine.getTextSystem(),
                "INVALID_STATE",
                "roboto",
                64);
        }

        _state = state;
        
        handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
    }
}