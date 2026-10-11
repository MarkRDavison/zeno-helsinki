#include <Scenes/GameEngineScene.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <Scenes/TitleEngineScene.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <GLFW/glfw3.h>

namespace slr
{

	GameEngineScene::GameEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig)
		: EngineScene(engine)
		, _engineConfig(engineConfig)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	GameEngineScene::~GameEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void GameEngineScene::initialise(
		const std::string& cameraMatrixResourceId,
		hl::VulkanDevice& device,
		hl::VulkanSwapChain& swapChain,
		hl::VulkanCommandPool& graphicsCommandPool,
		hl::VulkanCommandPool& transferCommandPool,
		hl::ResourceManager& resourceManager)
	{
		std::vector<hl::RenderpassInfo> renderpasses{
			hl::RenderGraphHelpers::createTextRenderpassInfo(cameraMatrixResourceId)
		};

		hl::ResourceContext resourceContext{
			.device = &device,
			.pool = &transferCommandPool,
			.resourceManager = &resourceManager,
			.materialSystem = &_engine.getMaterialSystem(),
			.rootPath = _engineConfig.RootPath
		};
		loadTextResources(resourceManager, resourceContext);

		_galaxy.initialise(_engine.getTextSystem(), _engineConfig.Width, _engineConfig.Height);

		registerPipelineDraw(
			"sdf_text_pipeline",
			[this](hl::PipelineDrawData& pdd)
			{
				_galaxy.draw(pdd, _engine.getTextSystem());
			});

		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);
	}

	void GameEngineScene::update(uint32_t, float)
	{
	}

	void GameEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_BACKSPACE)
			{
				_engine.setScene(new TitleEngineScene(_engine, _engineConfig));
			}
		}
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void GameEngineScene::handleWindowSizeChange(int width, int height)
	{
		_galaxy.layout(_engine.getTextSystem(), width, height);
	}

}
