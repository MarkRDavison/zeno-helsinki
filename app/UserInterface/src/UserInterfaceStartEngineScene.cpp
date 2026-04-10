#include "UserInterfaceStartEngineScene.hpp"
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>

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
            hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId)
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
    }

    UserInterfaceStartEngineScene::~UserInterfaceStartEngineScene()
    {
        _engine.getEventBus().RemoveListener(this);
    }

    void UserInterfaceStartEngineScene::update(uint32_t currentFrame, float delta)
    {

    }

    void UserInterfaceStartEngineScene::OnEvent(const hl::Event& event)
    {

    }

    void UserInterfaceStartEngineScene::handleWindowSizeChange(int width, int height)
    {

    }
}