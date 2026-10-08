#include <Scenes/Platformer3DEngineScene.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <Scenes/TitleEngineScene.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <GLFW/glfw3.h>

namespace phys
{
	Platformer3DEngineScene::Platformer3DEngineScene(
		hl::Engine& engine,
		const hl::EngineConfiguration& engineConfig,
		hl::physics::Context& physicsContext)
		: EngineScene(engine)
		, _engineConfig(engineConfig)
		, _physicsContext(physicsContext)
	{
		_cameras.insert({ "Default", new hl::Camera2D() });
		_engine.getEventBus().AddListener(this);
	}

	Platformer3DEngineScene::~Platformer3DEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void Platformer3DEngineScene::initialise(
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
		addTextEntity(_scene, _engine, "hint", "3D Platformer (stub)  Escape: title", 48);
		handleWindowSizeChange(_engineConfig.Width, _engineConfig.Height);
		EngineScene::initialise(
			cameraMatrixResourceId,
			device,
			swapChain,
			graphicsCommandPool,
			transferCommandPool,
			resourceManager,
			renderpasses);
	}

	void Platformer3DEngineScene::update(uint32_t, float)
	{
	}

	void Platformer3DEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto ke = dynamic_cast<const hl::KeyPressEvent*>(&event))
		{
			if (ke->GetKeyCode() == GLFW_KEY_ESCAPE)
			{
				_engine.setScene(new TitleEngineScene(_engine, _engineConfig, _physicsContext));
			}
		}
		else if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void Platformer3DEngineScene::handleWindowSizeChange(int width, int height)
	{
		centerTextAt(_scene, _engine, width, height, "hint", 0.0f);
	}
}
