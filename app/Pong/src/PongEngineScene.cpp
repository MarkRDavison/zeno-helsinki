#include "PongEngineScene.hpp"
#include "PongTitleEngineScene.hpp"
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/CubemapTextureResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/BasicModelResource.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <iostream>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <EntityPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/VertexArrayResource.hpp>
#include <Components/EntityComponent.hpp>
#include <Systems/PaddlePlayerControlSystem.hpp>
#include <Systems/PaddleComputerControlSystem.hpp>
#include <Systems/BallMovementSystem.hpp>
#include <Events/PointScoredEvent.hpp>
#include <helsinki/System/glm.hpp>
#include <GLFW/glfw3.h>
#include <helsinki/Engine/ECS/Components/KinematicComponent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>

namespace pong
{

    PongEngineScene::PongEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig)
    {
        _camera = new hl::Camera2D();
        _engine.getEventBus().AddListener(this);
    }


    PongEngineScene::~PongEngineScene()
    {
        _engine.getEventBus().RemoveListener(this);
    }

    void PongEngineScene::initialise(
        const std::string& cameraMatrixResourceId,
        hl::VulkanDevice& device,
        hl::VulkanSwapChain& swapChain,
        hl::VulkanCommandPool& graphicsCommandPool,
        hl::VulkanCommandPool& transferCommandPool,
        hl::ResourceManager& resourceManager,
        hl::MaterialSystem& materialSystem)
    {
        std::vector<hl::RenderpassInfo> renderpasses = 
        { 
            hl::RenderpassInfo
            {
                .name = "scene_pass",
                .useMultiSampling = false,
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
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/entity.frag"),
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
                                            .resource = "camera_matrix_ubo" // TODO: Constant
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
            },
            hl::RenderpassInfo
            {
                .name = "text_pass",
                .useMultiSampling = false,
                .inputs = {},
                .outputs =
                {
                    hl::ResourceInfo
                    {
                        .name = "text_color",
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
                            .name = "text_pipeline",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/text.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/text.frag"),
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
                                            .resource = "camera_matrix_ubo"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "VERTEX&FRAGMENT",
                                            .count = 64
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
                                        .offset = offsetof(hl::Vertex22D, pos)
                                    },
                                    {
                                        .name = "inTexCoord",
                                        .format = hl::VertexAttributeFormat::Vec2,
                                        .location = 1,
                                        .offset = offsetof(hl::Vertex22D, texCoord)
                                    }
                                },
                                .stride = sizeof(hl::Vertex22D)
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
                            .enableBlending = true, // keep blending for UI elements
                            .pushConstantSize = sizeof(hl::TextPushConstantObject)
                        },
                        hl::PipelineInfo
                        {
                            .name = "sdf_text_pipeline",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/text.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/sdf.frag"),
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
                                            .resource = "camera_matrix_ubo"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "VERTEX&FRAGMENT",
                                            .count = 64
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
                                        .offset = offsetof(hl::Vertex22D, pos)
                                    },
                                    {
                                        .name = "inTexCoord",
                                        .format = hl::VertexAttributeFormat::Vec2,
                                        .location = 1,
                                        .offset = offsetof(hl::Vertex22D, texCoord)
                                    }
                                },
                                .stride = sizeof(hl::Vertex22D)
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
                            .enableBlending = true, // keep blending for UI elements
                            .pushConstantSize = sizeof(hl::TextPushConstantObject)
                        }
                    }
                }
            },
            hl::RenderpassInfo
            {
                .name = "composite_pass",
                .useMultiSampling = false,
                .inputs = { "scene_color", "text_color" },
                .outputs =
                {
                    hl::ResourceInfo
                    {
                        .name = "swapchain_color",
                        .type = hl::ResourceType::Color,
                        .format = "VK_FORMAT_B8G8R8A8_SRGB"
                    }
                },
                .pipelineGroups =
                {
                    {
                        hl::PipelineInfo
                        {
                            .name = "composite_pipeline",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/fullscreen_sample.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/composite.frag"),
                            .descriptorSets =
                            {
                                hl::DescriptorSetInfo
                                {
                                    .name = "composite_inputs",
                                    .bindings =
                                    {
                                        hl::DescriptorBinding
                                        {
                                            .binding = 0,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "scene_color"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "text_color"
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
                            .enableBlending = true
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
            .materialSystem = &materialSystem,
            .rootPath = _engineConfig.RootPath
        };

        // TODO: Move to base and generate texture programatically
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            hl::MaterialSystem::FallbackTextureName,
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

        resourceManager.Load<hl::VertexArrayResource>(
            "wall",
            resourceContext,
            std::vector<hl::Vertex2>
            {
                { .pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::GameBoundsWidth, 0.0f } },
                { .pos = { (float)PongConstants::GameBoundsWidth, (float)PongConstants::GameBoundsWallWidth } },

                { .pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::GameBoundsWidth, (float)PongConstants::GameBoundsWallWidth } },
                { .pos = { 0.0f, (float)PongConstants::GameBoundsWallWidth } }
            });

        resourceManager.Load<hl::VertexArrayResource>(
            "paddle",
            resourceContext,
            std::vector<hl::Vertex2>
            {
                {.pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::PaddleWidth, 0.0f } },
                { .pos = { (float)PongConstants::PaddleWidth, (float)PongConstants::PaddleHeight } },

                { .pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::PaddleWidth, (float)PongConstants::PaddleHeight } },
                { .pos = { 0.0f, (float)PongConstants::PaddleHeight } },
            });

        resourceManager.Load<hl::VertexArrayResource>(
            "ball",
            resourceContext,
            std::vector<hl::Vertex2>
            {
                { .pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::BallSize, 0.0f } },
                { .pos = { (float)PongConstants::BallSize, (float)PongConstants::BallSize } },

                { .pos = { 0.0f, 0.0f } },
                { .pos = { (float)PongConstants::BallSize, (float)PongConstants::BallSize } },
                { .pos = { 0.0f, (float)PongConstants::BallSize } },
            });

        {
            auto entity = _scene.addEntity("Paddle1");
            entity->AddTag("ENTITY");
            entity->AddTag("PADDLE");
            entity->AddComponent<hl::TransformComponent>();
            auto ec = entity->AddComponent<EntityComponent>();
            ec->Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            ec->VertexBufferResourceName = "paddle";
            ec->ControlledState = ControlledState::PLAYER;
        }
        {
            auto entity = _scene.addEntity("Paddle2");
            entity->AddTag("ENTITY");
            entity->AddTag("PADDLE");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3((float)(PongConstants::GameBoundsWidth - PongConstants::PaddleWidth), 0.0f, 0.0f));
            auto ec = entity->AddComponent<EntityComponent>();
            ec->Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            ec->VertexBufferResourceName = "paddle";
            ec->ControlledState = ControlledState::COMPUTER;
        }
        {
            auto entity = _scene.addEntity("WallTop");
            entity->AddTag("ENTITY");
            entity->AddTag("WALL");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, (float)(-PongConstants::GameBoundsWallWidth), 0.0f));
            auto ec = entity->AddComponent<EntityComponent>();
            ec->Color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            ec->VertexBufferResourceName = "wall";
        }
        {
            auto entity = _scene.addEntity("WallBottom");
            entity->AddTag("ENTITY");
            entity->AddTag("WALL");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0f, (float)(PongConstants::GameBoundsHeight), 0.0f));
            auto ec = entity->AddComponent<EntityComponent>();
            ec->Color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
            ec->VertexBufferResourceName = "wall";
        }
        {
            auto entity = _scene.addEntity("Ball");
            entity->AddTag("ENTITY");
            entity->AddTag("BALL");
            entity->AddComponent<hl::TransformComponent>();
            entity->AddComponent<hl::KinematicComponent>();
            auto ec = entity->AddComponent<EntityComponent>();
            ec->Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            ec->VertexBufferResourceName = "ball";
        }
        {
            auto entity = _scene.addEntity("score1");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "0",
                "roboto",
                96);
            entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        }
        {
            auto entity = _scene.addEntity("score2");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>();
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(),
                "0",
                "roboto",
                96);
            entity->GetComponent<hl::TextComponent>()->setColour(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        }

        registerPipelineDraw(
            "entity_pipeline", 
            [&](hl::PipelineDrawData& pdd) -> void 
            {
                for (const auto& entity : pdd.scene->getEntities())
                {
                    if (!entity->HasComponents<hl::TransformComponent, EntityComponent>())
                    {
                        continue;
                    }

                    auto transform = entity->GetComponent<hl::TransformComponent>();
                    const auto& ec = entity->GetComponent<EntityComponent>();

                    const auto& resource = _resourceManager->GetResource<hl::VertexArrayResource>(ec->VertexBufferResourceName);

                    const auto& modelTransform = transform->GetTransformMatrix();

                    auto pc = EntityPushConstantObject
                    {
                        .model = glm::translate(modelTransform, glm::vec3(PongConstants::Offset, PongConstants::Offset, 0.0f)),
                        .color = ec->Color
                    };

                    vkCmdPushConstants(
                        pdd.commandBuffer,
                        pdd.pipeline->getPipelineLayout(),
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(EntityPushConstantObject),
                        &pc
                    ); 
                    VkBuffer vertexBuffers[] = { resource->_vertexBuffer._buffer };
                    VkDeviceSize offsets[] = { 0 };
                    vkCmdBindVertexBuffers(
                        pdd.commandBuffer,
                        0,
                        1,
                        vertexBuffers,
                        offsets);

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

        _scene.addSystem(new BallMovementSystem(
            _engine.getInputManager(), 
            _engine.getEventBus(),
            this->_scene));
        _scene.addSystem(new PaddlePlayerControlSystem(
            _engine.getInputManager(),
            this->_scene));
        _scene.addSystem(new PaddleComputerControlSystem(
            _engine.getInputManager(),
            this->_scene));

        _player1Score = 0;
        _player2Score = 0;

        handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
    }

    void PongEngineScene::update(uint32_t /*currentFrame*/, float delta)
    {
        if (_state == GameState::INIT)
        {
            auto paddle1 = _scene.getEntity("Paddle1");
            auto paddle2 = _scene.getEntity("Paddle2");

            auto tc1 = paddle1->GetComponent<hl::TransformComponent>();
            tc1->SetPosition(glm::vec3(
                tc1->GetPosition().x, 
                (float)(PongConstants::GameBoundsHeight - PongConstants::PaddleHeight) / 2.0f, 
                0.0f));

            auto tc2 = paddle2->GetComponent<hl::TransformComponent>();
            tc2->SetPosition(glm::vec3(
                tc2->GetPosition().x, 
                (float)(PongConstants::GameBoundsHeight - PongConstants::PaddleHeight) / 2.0f, 
                0.0f));

            auto ball = _scene.getEntity("Ball");
            ball->GetComponent<hl::KinematicComponent>()->velocity = 4.0f * glm::vec3(64.0f, 64.0f, 0.0f);
            auto btc = ball->GetComponent<hl::TransformComponent>();
            btc->SetPosition(glm::vec3(
                (float)(PongConstants::GameBoundsWidth - PongConstants::BallSize) / 2.0f,
                (float)(PongConstants::GameBoundsHeight - PongConstants::BallSize) / 2.0f,
                0.0f));

            _state = GameState::PLAYING;
        }
        else if (_state == GameState::PLAYING)
        {
            _scene.update(delta);
        }
        else if (_state == GameState::POINT_SCORED)
        {
            if (_engine.getInputManager().isKeyDown(GLFW_KEY_ENTER))
            {
                _state = GameState::INIT;
            }
        }
    }

    void PongEngineScene::OnEvent(const hl::Event& event)
    {
        // TODO: Better way to do this without the dynamic cast?
        // Maybe a type property that can be compared?
        if (auto pse = dynamic_cast<const PointScoredEvent*>(&event))
        {
            const auto player = pse->GetPlayerNumber();
            std::cout << "Player '" << player << "' scored!" << std::endl;
            if (player == 1)
            {
                _player1Score += 1; 
                auto entity = _scene.getEntity("score1");
                entity->GetComponent<hl::TextComponent>()->setString(_engine.getTextSystem(),
                    std::to_string(_player1Score),
                    "roboto",
                    96);
            }
            else
            {
                _player2Score += 1; 
                auto entity = _scene.getEntity("score2");
                entity->GetComponent<hl::TextComponent>()->setString(_engine.getTextSystem(),
                    std::to_string(_player2Score),
                    "roboto",
                    96);
            }

            handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
            
            _state = GameState::POINT_SCORED;
        } 
        else if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
        {
            const auto code = ke->GetKeyCode();

            if (code == GLFW_KEY_BACKSPACE)
            {
                std::cout << "BACKSPACE PRESSED!" << std::endl;
                _engine.setScene(new PongTitleEngineScene(_engine, _engineConfig));
            }
        }
        else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
        {
            handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
        }
    }

    // TODO: Break out into handle ui layout etc....
    void PongEngineScene::handleWindowSizeChange(int width, int height)
    {
        const auto& alignTextTopLeft = [&](const std::string& entityName) -> void
        {
            auto desiredPosition = glm::vec2();

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredPosition, 0.0f));
        };
        
        const auto& alignTextTopRight = [&](const std::string& entityName) -> void
        {
            auto desiredPosition = glm::vec2(width, 0.0f);

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            desiredPosition.x += size.x - size.z;

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredPosition, 0.0f));
        };
        const auto& centerTextAt = [&](const std::string& entityName, float yOffset) -> void
        {
            auto desiredCenter = glm::vec2((float)width / 2.0f, yOffset);

            auto entity = _scene.getEntity(entityName);

            const auto& size = _engine
                .getTextSystem()
                .getTextSize(
                    entity->GetComponent<hl::TextComponent>()->getTextSystemId());

            desiredCenter.x += size.x - size.z / 2.0f;
            desiredCenter.y += size.y - size.w / 2.0f;

            entity->GetComponent<hl::TransformComponent>()->SetPosition(glm::vec3(desiredCenter, 0.0f));
        };

        alignTextTopLeft("score1");
        alignTextTopRight("score2");
    }
}