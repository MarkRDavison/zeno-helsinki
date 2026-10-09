#include <Scenes/DrillerGameEngineScene.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>

namespace drl
{

	DrillerGameEngineScene::DrillerGameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		Session& session
	) :
		EngineScene(engine),
		_engineConfig(engineConfig),
		_session(session)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
	}

	void DrillerGameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		std::vector<hl::RenderpassInfo> renderpasses
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
						.clear = VkClearValue{.color = { 0.0f, 0.2f, 0.8f, 1.0f }}
					}
				},
				.pipelineGroups = {}
			}
		};

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);
	}

	void DrillerGameEngineScene::update(uint32_t /*currentFrame*/, float /*delta*/)
	{
	}

}
