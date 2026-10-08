#include <Scenes/TitleEngineScene.hpp>
#include <Scenes/Platformer2DEngineScene.hpp>
#include <Scenes/Platformer3DEngineScene.hpp>
#include <Scenes/TextMenuSupport.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraphHelpers.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Infrastructure/Camera2D.hpp>
#include <GLFW/glfw3.h>

namespace phys
{
	TitleEngineScene::TitleEngineScene(
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

	TitleEngineScene::~TitleEngineScene()
	{
		_engine.getEventBus().RemoveListener(this);
	}

	void TitleEngineScene::initialise(
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

		addTextEntity(_scene, _engine, "title", "PHYSICS", 128, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
		addTextEntity(_scene, _engine, "platformer2d", "2D Platformer", 64);
		addTextEntity(_scene, _engine, "platformer3d", "3D Platformer", 64);
		addTextEntity(_scene, _engine, "quit", "Quit", 64);

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

	void TitleEngineScene::update(uint32_t, float)
	{
		const auto mouse = _engine.getInputManager().getMousePosition();
		const auto checkClick = _engine.getInputManager().isButtonReleased(GLFW_MOUSE_BUTTON_1);

		for (auto& e : _scene.getEntitiesWithComponents<hl::TransformComponent, hl::TextComponent>("TEXT"))
		{
			if (e->getName() == "title")
			{
				continue;
			}

			auto tc = e->GetComponent<hl::TransformComponent>();
			auto textComponent = e->GetComponent<hl::TextComponent>();
			const auto tcp = tc->GetPosition();
			const auto& size = _engine.getTextSystem().getTextSize(textComponent->getTextSystemId());

			if ((tcp.x + size.x <= mouse.x) && (mouse.x <= tcp.x + size.z) &&
				(tcp.y + size.y <= mouse.y) && (mouse.y <= tcp.y + size.w))
			{
				textComponent->setColour(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
				if (checkClick)
				{
					handleTextClicked(e->getName());
				}
			}
			else
			{
				textComponent->setColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}
		}
	}

	void TitleEngineScene::OnEvent(const hl::Event& event)
	{
		if (auto wre = dynamic_cast<const hl::WindowResizeEvent*>(&event))
		{
			handleWindowSizeChange(wre->GetWidth(), wre->GetHeight());
		}
	}

	void TitleEngineScene::handleWindowSizeChange(int width, int height)
	{
		centerTextAt(_scene, _engine, width, height, "title", 0.0f);
		centerTextAt(_scene, _engine, width, height, "platformer2d", 1.0f * height / 8.0f);
		centerTextAt(_scene, _engine, width, height, "platformer3d", 2.0f * height / 8.0f);
		centerTextAt(_scene, _engine, width, height, "quit", 3.0f * height / 8.0f);
	}

	void TitleEngineScene::handleTextClicked(const std::string& name)
	{
		if (name == "quit")
		{
			_engine.stop();
		}
		else if (name == "platformer2d")
		{
			_engine.setScene(new Platformer2DEngineScene(_engine, _engineConfig, _physicsContext));
		}
		else if (name == "platformer3d")
		{
			_engine.setScene(new Platformer3DEngineScene(_engine, _engineConfig, _physicsContext));
		}
	}
}
