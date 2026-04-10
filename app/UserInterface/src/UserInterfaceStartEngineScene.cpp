#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

#define VERTEX_COUNT 6

namespace ui
{

    UserInterfaceStartEngineScene::UserInterfaceStartEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig)
    {
        _camera = new hl::Camera2D();
        _engine.getEventBus().AddListener(this);
    }

    UserInterfaceStartEngineScene::~UserInterfaceStartEngineScene()
    {
        _engine.getEventBus().RemoveListener(this);
    }


    void UserInterfaceStartEngineScene::initialise(
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
                        .clear = VkClearValue{.color = { 0.0f, 0.2f, 0.8f, 1.0f}}
                    },
                    hl::ResourceInfo
                    {
                        .name = "ui_depth",
                        .type = hl::ResourceType::Depth,
                        .format = "VK_FORMAT_D32_SFLOAT"
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
                                            .resource = cameraMatrixResourceId
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
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "white",
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

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            _mappedBuffers.emplace_back(device);
            _mappedBuffers.back().create(sizeof(hl::Vertex2) * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }

        registerPipelineDraw(
            "ui_pipeline",
            [&](hl::PipelineDrawData& pdd) -> void
            {
                VkBuffer vertexBuffers[] = { _mappedBuffers[pdd.currentFrame].getBuffer() };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(
                    pdd.commandBuffer,
                    0,
                    1,
                    vertexBuffers,
                    offsets);

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

                vkCmdDraw(pdd.commandBuffer, VERTEX_COUNT, 1, 0, 0);
            });
    }

    void UserInterfaceStartEngineScene::update(uint32_t currentFrame, float delta)
    {
        std::array<hl::Vertex2, VERTEX_COUNT> verts
        {
            hl::Vertex2{.pos = { 100.0f, 200.0f + 5.0f } },
            hl::Vertex2{.pos = { 200.0f, 200.0f + 5.0f } },
            hl::Vertex2{.pos = { 200.0f, 100.0f + 5.0f } },

            hl::Vertex2{.pos = { 100.0f, 200.0f - 5.0f } },
            hl::Vertex2{.pos = { 200.0f, 100.0f - 5.0f } },
            hl::Vertex2{.pos = { 100.0f, 100.0f - 5.0f } },
        };

        _mappedBuffers[currentFrame].write(verts.data(), sizeof(hl::Vertex2) * VERTEX_COUNT);
    }

    void UserInterfaceStartEngineScene::additionalCleanup()
    {
        for (auto& mb : _mappedBuffers)
        {
            mb.destroy();
        }

        _mappedBuffers.clear();
    }

    void UserInterfaceStartEngineScene::OnEvent(const hl::Event& event)
    {

    }

    void UserInterfaceStartEngineScene::handleWindowSizeChange(int width, int height)
    {

    }
}