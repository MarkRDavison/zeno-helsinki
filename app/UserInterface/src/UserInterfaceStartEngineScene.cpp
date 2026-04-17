#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/SignedDistanceFieldFontResource.hpp>

#define MAX_UI_TEXTURES 64

namespace ui
{

    UserInterfaceStartEngineScene::UserInterfaceStartEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig),
        _uiRoot(engine.getInputManager(), engine.getResourceManager())
    {
        _camera = new hl::Camera2D();
        _engine.getEventBus().AddListener(this);
        _engine.getEventBus().AddListener(&_uiRoot);
    }

    UserInterfaceStartEngineScene::~UserInterfaceStartEngineScene()
    {
        _engine.getEventBus().RemoveListener(&_uiRoot);
        _engine.getEventBus().RemoveListener(this);
    }

    struct NewUiSceneVertexInputAttributes
    {
        hl::Vec2f position;
        hl::Vec2f uv;
        hl::Vec4f colour;
    };

    struct alignas(16) NewUiScenePushConstants
    {
        uint32_t texId{ 0 };
        uint32_t RenderType{ 0 };
    };

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
            // TODO: Move to render graph helpers
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
                        .clear = VkClearValue{ .color = { 0.0f, 0.2f, 0.8f, 1.0f} }
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
                                        },
                                        hl::DescriptorBinding
                                        {
                                            .binding = 1,
                                            .type = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                                            .stage = "FRAGMENT",
                                            .count = MAX_UI_TEXTURES
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
                                        .name = "inTexUv",
                                        .format = hl::VertexAttributeFormat::Vec2,
                                        .location = 1,
                                        .offset = offsetof(hl::VertexUi, texCoord)
                                    },
                                    {
                                        .name = "inColor",
                                        .format = hl::VertexAttributeFormat::Vec3,
                                        .location = 2,
                                        .offset = offsetof(hl::VertexUi, color)
                                    },
                                    {
                                        .name = "inRenderType",
                                        .format = hl::VertexAttributeFormat::Uint,
                                        .location = 3,
                                        .offset = offsetof(hl::VertexUi, renderType)
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
                            .enableBlending = true,
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
        resourceManager.LoadAs<hl::SignedDistanceFieldFontResource, hl::FontResource>(
            "roboto",
            resourceContext);
        resourceManager.LoadAs<hl::TextureResource, hl::ImageSamplerResource>(
            "roboto",
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

        _uiRoot.initialise(device, *_renderGraph, resourceManager);

        registerPipelineDraw("ui_pipeline", [&](hl::PipelineDrawData& pdd) -> void { _uiRoot.draw(pdd); });
    }

    void UserInterfaceStartEngineScene::update(uint32_t currentFrame, float delta)
    {
        _uiRoot.update(delta);
    }
    void UserInterfaceStartEngineScene::updateGpuResources(uint32_t currentFrame)
    {
        _uiRoot.updateGpuResources(currentFrame);
    }

    void UserInterfaceStartEngineScene::additionalCleanup()
    {
        _uiRoot.destroy();
    }

    void UserInterfaceStartEngineScene::OnEvent(const hl::Event& event)
    {

    }

    void UserInterfaceStartEngineScene::handleWindowSizeChange(int width, int height)
    {

    }
}