#pragma once

#include <helsinki/Engine/EngineConfiguration.hpp>
#include <helsinki/Engine/EngineScene.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/FrameResources.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSynchronisationContext.hpp>
#include <helsinki/Engine/Input/InputManager.hpp>

struct GLFWwindow;

namespace hl
{

	class Engine : NonCopyable
	{
	public:
		Engine(EventBus& eventBus, InputManager& inputManager);
		~Engine();

		void init(EngineConfiguration config);
		void run();
		void notifyFramebufferResized(int width, int height);
		void sendEvent(const Event& event);

		void setScene(EngineScene* scene);
		void stop();
		void setFullscreen(bool fullscreen);
		bool isFullscreen() const;
		void setVsync(bool enable);
		bool isVsync() const;

		TextSystem& getTextSystem() { return _textSystem; }
		MaterialSystem& getMaterialSystem() { return _materialSystem; }
		InputManager& getInputManager() { return _inputManager; }
		EventBus& getEventBus() { return _eventBus; }

	private:
		void mainLoop();
		void cleanup();
		void update(float delta);
		void draw();

		void initWindow(uint32_t width, uint32_t height, const char* title);
		void initVulkan(const char* title);
		void recreateSwapChain();

		void createScene(EngineScene* scene);
		void destroyScene();

		void setCurrentSceneAsAppropriate();

	private:
		EventBus& _eventBus;
		InputManager& _inputManager;
		VulkanInstance _instance;
		VulkanSurface _surface;
		VulkanDevice _device;
		VulkanCommandPool _graphicsCommandPool;
		VulkanCommandPool _transferCommandPool;
		VulkanSwapChain _swapChain;
		VulkanSynchronisationContext _syncContext;

		ResourceManager _resourceManager;
		MaterialSystem _materialSystem;
		TextSystem _textSystem;

		EngineConfiguration _config;
		uint32_t _currentFrame = 0;
		bool _framebufferResized = false;
		bool _running = false;

		std::unique_ptr<EngineScene> _currentEngineScene;
		EngineScene* _nextEngineScene{nullptr};

		GLFWwindow* _window{ nullptr };
		int _windowedX = 100;
		int _windowedY = 100;
		int _windowedWidth = 800;
		int _windowedHeight = 600;


	};

}