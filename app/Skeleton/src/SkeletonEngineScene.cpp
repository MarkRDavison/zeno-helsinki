#include "SkeletonEngineScene.hpp"
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
#include <iostream>

namespace sk
{

    SkeletonEngineScene::SkeletonEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig)
    {

    }
	void SkeletonEngineScene::initialise(
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
                .useMultiSampling = true,
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
                            .name = "skybox_pipeline",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/skybox.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/skybox.frag"),
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
                                            .resource = "camera_matrix_ubo"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "skybox_texture"
                                        }
                                    }
                                }
                            },
                            .depthState =
                            {
                                .writeEnable = false,
                                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
                            },
                            .rasterState =
                            {
                                .cullMode = VK_CULL_MODE_NONE
                            },
                            .enableBlending = false,
                            .pushConstantSize = sizeof(hl::MaterialPushConstantObject)
                        },
                    },
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
                                            .stage = "VERTEX",
                                            .resource = "camera_matrix_ubo"
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
                .useMultiSampling = false,
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
                .useMultiSampling = false,
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
                            .name = "ui",
                            .shaderVert = _engineConfig.RootPath + std::string("/data/shaders/ui.vert"),
                            .shaderFrag = _engineConfig.RootPath + std::string("/data/shaders/ui.frag"),
                            .descriptorSets = {},
                            .depthState =
                            {
                                .testEnable = false,
                                .writeEnable = false
                            },
                            .rasterState =
                            {
                                .cullMode = VK_CULL_MODE_NONE
                            },
                            .enableBlending = true // keep blending for UI elements
                        }
                    }
                }
            },
            hl::RenderpassInfo
            {
                .name = "composite_pass",
                .useMultiSampling = false,
                .inputs = { "post_color", "ui_color" },
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
                                            .resource = "post_color"
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .resource = "ui_color"
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
            hl::MaterialSystem::FallbackTextureName,
            resourceContext);
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "white",
            resourceContext);
        resourceManager.LoadAs<hl::CubemapTextureResource, hl::ImageSamplerResource>(
            "skybox_texture",
            resourceContext);

        {
            auto planeModelHandle = resourceManager.Load<hl::ModelResource>(
                "plane",
                resourceContext);

            auto plane = _scene.addEntity(planeModelHandle->GetId());
            plane->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(0.0, 0.0, 0.0));
            plane->AddComponent<hl::ModelComponent>()->setModelId(planeModelHandle->GetId());
        }
        {
            auto rockCrystalsModelHandle = resourceManager.Load<hl::ModelResource>(
                "rock_crystals",
                resourceContext);

            auto satellite = _scene.addEntity(rockCrystalsModelHandle->GetId());
            satellite->AddTag("ROTATE");
            satellite->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(-1.0, 0.0, -1.0));
            satellite->AddComponent<hl::ModelComponent>()->setModelId(rockCrystalsModelHandle->GetId());
        }
        {
            auto satelliteModelHandle = resourceManager.Load<hl::ModelResource>(
                "satelliteDish_detailed",
                resourceContext);

            auto satellite = _scene.addEntity(satelliteModelHandle->GetId());
            satellite->AddTag("ROTATE");
            satellite->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(-1.0, 0.0, +1.0));
            satellite->AddComponent<hl::ModelComponent>()->setModelId(satelliteModelHandle->GetId());
        }
        {
            auto turrentModelHandle = resourceManager.Load<hl::ModelResource>(
                "turret_double",
                resourceContext);

            auto turret = _scene.addEntity(turrentModelHandle->GetId());
            turret->AddTag("ROTATE");
            turret->AddComponent<hl::TransformComponent>()->SetPosition(glm::vec3(+1.0, 0.0, +1.0));
            turret->AddComponent<hl::ModelComponent>()->setModelId(turrentModelHandle->GetId());
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

    void SkeletonEngineScene::update(uint32_t currentFrame, float delta)
    {
        static float angle = 0.0f;

        angle += 45.0f * delta;

        for (auto& e : _scene.getEntities())
        {
            if (e->HasComponents<hl::TransformComponent, hl::ModelComponent>() &&
                e->HasTag("ROTATE"))
            {
                auto transform = e->GetComponent<hl::TransformComponent>();
                transform->SetRotation(glm::quat(glm::vec3(0.0, glm::radians(angle), 0.0)));
            }
        }
    }
}