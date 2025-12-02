#include <TwoDimensionalEngineScene.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/SpritePushConstantObject.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/TextPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>
#include <helsinki/Renderer/Resource/RasterisedFontResource.hpp>
#include <helsinki/Renderer/Resource/CubemapTextureResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Resource/BasicModelResource.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Resource/FrameDataStorageBufferObject.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/SpriteComponent.hpp>
#include <helsinki/Engine/ECS/Components/TextComponent.hpp>
#include <iostream>

namespace td
{

	TwoDimensionalEngineScene::TwoDimensionalEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig)
    {
        _camera = new hl::Camera2D();
    }

    void TwoDimensionalEngineScene::initialise(
        const std::string& cameraMatrixResourceId,
        hl::VulkanDevice& device,
        hl::VulkanSwapChain& swapChain,
        hl::VulkanCommandPool& graphicsCommandPool,
        hl::VulkanCommandPool& transferCommandPool,
        hl::ResourceManager& resourceManager,
        hl::MaterialSystem& materialSystem)
    {
        std::vector<hl::RenderpassInfo> renderpasses
        {
            hl::RenderpassInfo
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
                                            .resource = "camera_matrix_ubo"
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
                                            .resource = "spritesheet"
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
                                            .stage = "FRAGMENT"
                                            //.resource = "consola" // Setting this ourselves in the pipeline render method...
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
                                            .stage = "FRAGMENT"
                                            //.resource = "consola" // Setting this ourselves in the pipeline render method...
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

        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "spritesheet",
            resourceContext);

        resourceManager.LoadAs<hl::RasterisedFontResource, hl::FontResource>("consolab", resourceContext);
        resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>("consola", resourceContext);
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "consola",
            resourceContext);
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "consolab",
            resourceContext);

        frameSSBOResourceHandle = resourceManager.Load<hl::StorageBufferResource>(
            "spritesheet_frame_ssbo",
            resourceContext,
            sizeof(hl::FrameDataStorageBufferObject),
            128);

        {
            const constexpr float TEX_SIZE = 1024.0f;
            const constexpr float CELL_SIZE = 64.0f;
            auto ssbo = frameSSBOResourceHandle.Get();
            std::vector<hl::FrameDataStorageBufferObject> frameData
            {
                {.uvRect = glm::vec4(CELL_SIZE * 0, CELL_SIZE * 0, CELL_SIZE * 1, CELL_SIZE * 1) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 1, CELL_SIZE * 0, CELL_SIZE * 2, CELL_SIZE * 1) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 2, CELL_SIZE * 0, CELL_SIZE * 3, CELL_SIZE * 1) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 0, CELL_SIZE * 1, CELL_SIZE * 1, CELL_SIZE * 2) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 1, CELL_SIZE * 1, CELL_SIZE * 2, CELL_SIZE * 2) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 2, CELL_SIZE * 1, CELL_SIZE * 3, CELL_SIZE * 2) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 0, CELL_SIZE * 2, CELL_SIZE * 1, CELL_SIZE * 3) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 1, CELL_SIZE * 2, CELL_SIZE * 2, CELL_SIZE * 3) / TEX_SIZE },
                {.uvRect = glm::vec4(CELL_SIZE * 2, CELL_SIZE * 2, CELL_SIZE * 3, CELL_SIZE * 3) / TEX_SIZE },
            };

            for (uint32_t i = 0; i < (uint32_t)frameData.size(); ++i)
            {
                ssbo->writeToBuffer(&frameData[i], i);
            }
        }

        {
            auto entity = _scene.addEntity("entity1");
            entity->AddTag("SPRITE");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(64.0f, 128.0f, 0.0f));
            entity->AddComponent<hl::SpriteComponent>()->setFrameDataIndex(0);
        }
        {
            auto entity = _scene.addEntity("entity2");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(192.0f, 128.0f, 0.0f));
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(), 
                "SDF text - '? JQjqPp <>", 
                "consola",
                96);
        }
        {
            auto entity = _scene.addEntity("entity3");
            entity->AddTag("TEXT");
            entity->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(192.0f, 256.0f, 0.0f));
            // TODO: Dont like having to pass text system here...
            entity->AddComponent<hl::TextComponent>()->setString(
                _engine.getTextSystem(), 
                "Rasterised Text - #!@# 9JpqQ", 
                "consolab",
                24);
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
    }

    void TwoDimensionalEngineScene::update(uint32_t /*currentFrame*/, float delta)
    {
        const int max = 9;
        static float acc = 0.0f;

        acc += delta;
        if (acc > 1.0f)
        {
            acc -= 1.0f;
            for (auto& e : _scene.getEntities())
            {
                if (e->HasTag("SPRITE") &&
                    e->HasComponent<hl::SpriteComponent>())
                {
                    auto sc = e->GetComponent<hl::SpriteComponent>();
                    auto curr = sc->getFrameDataIndex();
                    auto next = (curr + 1) % max;
                    sc->setFrameDataIndex(next);
                }
            }
        }

    }
}