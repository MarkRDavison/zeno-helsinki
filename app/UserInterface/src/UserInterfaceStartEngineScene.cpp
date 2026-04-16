#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

namespace ui
{

    UserInterfaceStartEngineScene::UserInterfaceStartEngineScene(
        hl::Engine& engine,
        const hl::EngineConfiguration& engineConfig
    ) :
        EngineScene(engine),
        _engineConfig(engineConfig),
        _uiRoot(engine.getInputManager())
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

        _uiRoot.initialise(device);

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