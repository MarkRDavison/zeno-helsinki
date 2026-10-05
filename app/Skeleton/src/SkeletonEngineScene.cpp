#include "SkeletonEngineScene.hpp"
#include <SceneCatalog.hpp>
#include <SunUniformBufferObject.hpp>
#include <PointLightsUniformBufferObject.hpp>
#include <Systems/RotateSystem.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/MaterialPushConstantObject.hpp>
#include <helsinki/Renderer/Resource/CubemapTextureResource.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ModelResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/RendererShaderRoot.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace sk
{

    SkeletonEngineScene::SkeletonEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig)
    {
        // EngineScene destructor deletes cameras stored in _cameras.
        _cameras.insert({ "Default", new hl::Camera(
            glm::vec3(2.0f, 0.5f, -2.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            135.0f,
            -5.0f) });
    }

    std::vector<hl::RenderpassInfo> SkeletonEngineScene::buildRenderpasses() const
    {
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
                                            .stage = "VERTEX&FRAGMENT",
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
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 3,
                                            .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                            .stage = "FRAGMENT",
                                            .resource = "sun_ubo",
                                            .count = 1,
                                            .updateFrequency = hl::DescriptorUpdateFrequency::PerFrame
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 4,
                                            .type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                                            .stage = "FRAGMENT",
                                            .resource = "point_lights_ubo",
                                            .count = 1,
                                            .updateFrequency = hl::DescriptorUpdateFrequency::PerFrame
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

    void SkeletonEngineScene::spawnScene(
        hl::ResourceManager& resourceManager,
        hl::ResourceContext& resourceContext)
    {
        resourceManager.LoadAs<hl::CubemapTextureResource, hl::ImageSamplerResource>(
            "skybox_texture",
            resourceContext);

        _sunUbo = resourceManager.Load<hl::UniformBufferResource>(
            "sun_ubo",
            resourceContext,
            sizeof(SunUniformBufferObject),
            MAX_FRAMES_IN_FLIGHT,
            1);

        _pointLightsUbo = resourceManager.Load<hl::UniformBufferResource>(
            "point_lights_ubo",
            resourceContext,
            sizeof(PointLightsUniformBufferObject),
            MAX_FRAMES_IN_FLIGHT,
            1);

        const hl::ResourceDefinition uiSheetDefinition
        {
            .name = "ui_sheet",
            .type = "logical",
            .resources =
            {
                hl::ResourceDefinition::Child
                {
                    .name = hl::MaterialSystem::FallbackTextureName,
                    .type = "texture"
                }
            }
        };

        resourceManager.LoadLogical(uiSheetDefinition, [&](const hl::ResourceDefinition::Child& child)
            {
                if (child.type != "texture")
                {
                    return false;
                }

                if (!resourceManager.HasResource<hl::ImageSamplerResource>(child.name))
                {
                    resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
                        child.name,
                        resourceContext);
                }

                return resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
            });

        for (const auto& prop : SceneProps)
        {
            auto modelHandle = resourceManager.Load<hl::ModelResource>(
                prop.modelId,
                resourceContext);

            auto* entity = _scene.addEntity(modelHandle->GetId());
            if (prop.rotate)
            {
                entity->AddTag(RotateTag);
            }
            entity->AddComponent<hl::TransformComponent>()->SetPosition(prop.position);
            entity->AddComponent<hl::ModelComponent>()->setModelId(modelHandle->GetId());
        }

        _scene.addSystem(new RotateSystem(_scene));
    }

	void SkeletonEngineScene::initialise(
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

        registerPipelineDraw("ui_pipeline", [](hl::PipelineDrawData&) {});
	}

    void SkeletonEngineScene::update(uint32_t /*currentFrame*/, float delta)
    {
        _specHeldOff = _engine.getInputManager().isKeyDown(GLFW_KEY_H);
        _scene.update(delta);
    }

    void SkeletonEngineScene::updateGpuResources(uint32_t currentFrame)
    {
        if (_sunUbo)
        {
            SunUniformBufferObject ubo{};
            const auto dir = glm::normalize(glm::vec3(0.45f, 0.85f, 0.30f));
            ubo.direction = glm::vec4(dir, 1.0f);
            ubo.color = glm::vec4(1.0f, 0.97f, 0.90f, _specHeldOff ? 0.0f : 0.35f);
            ubo.ambient = glm::vec4(0.18f, 0.18f, 0.18f, 0.0f);
            _sunUbo.Get()->getUniformBuffer(currentFrame).writeToBuffer(&ubo, 0);
        }

        if (_pointLightsUbo)
        {
            PointLightsUniformBufferObject lights{};
            int n = static_cast<int>(sizeof(ScenePointLights) / sizeof(ScenePointLights[0]));
            n = std::min(n, MaxPointLights);
            lights.count.x = n;
            for (int i = 0; i < n; ++i)
            {
                const auto& l = ScenePointLights[i];
                lights.positionRadius[i] = glm::vec4(l.position, l.radius);
                lights.colorIntensity[i] = glm::vec4(l.color, l.intensity);
            }
            _pointLightsUbo.Get()->getUniformBuffer(currentFrame).writeToBuffer(&lights, 0);
        }
    }
}
