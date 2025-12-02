#include <helsinki/Engine/Engine.hpp>

#include <helsinki/System/HelsinkiTracy.hpp>
#include <helsinki/System/Events/WindowResizeEvent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <helsinki/System/Events/ScrollEvent.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/CameraUniformBufferObject.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace hl
{

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
		app->notifyFramebufferResized(width, height);
	}
	static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			KeyPressEvent event(key);
			app->sendEvent(event);
		}
		else if (action == GLFW_RELEASE)
		{
			KeyReleaseEvent event(key);
			app->sendEvent(event);
		}
	}
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
		ScrollEvent event((int)xOffset, (int)yOffset);
		app->sendEvent(event);
	}

	Engine::Engine(
		EventBus& eventBus
	) :
		_eventBus(eventBus),
		_window(nullptr),
		_instance(),
		_surface(_instance),
		_device(_instance, _surface),
		_swapChain(_device, _surface),
		_graphicsCommandPool(_device),
		_transferCommandPool(_device),
		_syncContext(_device),
		_resourceManager(),
		_materialSystem(_device, _resourceManager),
		_textSystem(_device, _transferCommandPool, _resourceManager)
	{

	}
	Engine::~Engine()
	{

	}

	void Engine::init(EngineConfiguration config)
	{
		_config = config;
		initWindow(_config.Width, _config.Height, _config.Title.c_str());
		initVulkan(_config.Title.c_str());
	}
	void Engine::run()
	{
		mainLoop();
		cleanup();
	}
	void Engine::notifyFramebufferResized(int width, int height)
	{
		_framebufferResized = true;
		// TODO: Should this be sent now or when we actually re-create the framebuffer/swapchain?
		WindowResizeEvent event(width, height);
		_eventBus.PublishEvent(event);
	}
	void Engine::sendEvent(const Event& event)
	{
		_eventBus.PublishEvent(event);
	}
	void Engine::setScene(EngineScene* scene)
	{
		if (_currentEngineScene)
		{
			destroyScene();
		}

		createScene(scene);

		{
			ZoneScopedN("updateAllOutputResources");
			scene->updateAllOutputResources();
		}

		{
			ZoneScopedN("updateAllDescriptorSets");
			scene->updateAllDescriptorSets();
		}
	}

	void Engine::mainLoop()
	{
		const float delta = 1.0f / 60.0f;
		float accumulator = 0.0f;
		float statsAccumulator = 0.0f;
		unsigned int fps = 0;
		unsigned int ups = 0;

		auto start = std::chrono::steady_clock::now();

		while (!glfwWindowShouldClose(_window))
		{
			ZoneScopedN("MainLoop");
			auto now = std::chrono::steady_clock::now();
			const auto frameTime = now - start;
			start = now;

			accumulator += std::chrono::duration<float>(frameTime).count();
			statsAccumulator += std::chrono::duration<float>(frameTime).count();

			if (statsAccumulator >= 1.0f)
			{
				if (_config.DisplayFps)
				{
					const auto title = std::format("{} - FPS: {} UPS: {}", _config.Title, fps, ups);
					glfwSetWindowTitle(_window, title.c_str());
				}
				statsAccumulator -= 1.0f;
				fps = 0;
				ups = 0;
			}

			while (accumulator >= delta)
			{
				glfwPollEvents();

				update(delta);
				ups++;

				accumulator -= delta;
			}

			draw();
			fps++;
		}

		_device.waitIdle();
	}
	void Engine::cleanup()
	{
		destroyScene();

		_textSystem.destroy();
		_materialSystem.destroy();

		_swapChain.destroy();

		_resourceManager.UnloadAll();

		_syncContext.destroy();
		_transferCommandPool.destroy();
		_graphicsCommandPool.destroy();
		_device.destroy();
		_surface.destroy();
		_instance.destroy();

		glfwDestroyWindow(_window);

		glfwTerminate();
	}
	void Engine::update(float delta)
	{
		ZoneScopedN("Update");
		_currentEngineScene->updateBase(_currentFrame, delta);
	}
	void Engine::draw()
	{
		ZoneScopedN("Draw");

		{
			ZoneScopedN("Wait fence");
			_syncContext.getFence(_currentFrame).wait();
		}

		VkResult result;
		uint32_t imageIndex;
		{
			ZoneScopedN("Acquire next image");
			result = vkAcquireNextImageKHR(
				_device._device,
				_swapChain._swapChain,
				UINT64_MAX,
				_syncContext.getImageAvailableSemaphore(_currentFrame)._semaphore,
				VK_NULL_HANDLE,
				&imageIndex);
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			ZoneScopedN("Recreate swapchain");
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		_syncContext.getFence(_currentFrame).reset();

		const auto primaryCommandBuffer = _currentEngineScene->draw(_currentFrame, imageIndex);

		VkSemaphore waitSemaphores[] = { _syncContext.getImageAvailableSemaphore(_currentFrame)._semaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { _syncContext.getRenderFinishedSemaphore(_currentFrame)._semaphore };
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &primaryCommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		{
			ZoneScopedN("Submit render queue");
			CHECK_VK_RESULT(vkQueueSubmit(
				_device._graphicsQueue._queue, 
				1, 
				&submitInfo, 
				_syncContext.getFence(_currentFrame)._fence));
		}

		VkSwapchainKHR swapChains[] = { _swapChain._swapChain };
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		{
			ZoneScopedN("Present queue");
			result = vkQueuePresentKHR(_device._presentQueue._queue, &presentInfo);
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized)
		{
			ZoneScopedN("Recreate swapchain");
			_framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Engine::initWindow(uint32_t width, uint32_t height, const char* title)
	{
		ZoneScopedN("InitWindow");
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
		glfwSetKeyCallback(_window, keyCallback);
		glfwSetScrollCallback(_window, scrollCallback);
	}
	void Engine::initVulkan(const char* title)
	{
		ZoneScopedN("InitVulkan");
		{
			{
				ZoneScopedN("Create Instance");
				_instance.create(title);
			}
			{
				ZoneScopedN("Create Surface");
				_surface.create(_window);
			}
			{
				ZoneScopedN("Create Device");
				_device.create();
			}
			{
				ZoneScopedN("Create swapchain");
				_swapChain.create(_config.EnableVsync);
			}
			{
				_graphicsCommandPool.create();
				_transferCommandPool.createTransferPool();
				_syncContext.create();
			}
			{
				ZoneScopedN("LoadResources");

				hl::ResourceContext resourceContext
				{
					.device = &_device,
					.pool = &_transferCommandPool,
					.resourceManager = &_resourceManager,
					.materialSystem = &_materialSystem,
					.rootPath = _config.RootPath,
				};

				_materialSystem.create(_config.MaxMaterials);

				_resourceManager.Load<hl::UniformBufferResource>(
					"camera_matrix_ubo",
					resourceContext,
					sizeof(CameraUniformBufferObject),
					MAX_FRAMES_IN_FLIGHT,
					1);
			}
		}
	}
	void Engine::recreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(_window, &width, &height);
			glfwWaitEvents();
		}

		_device.waitIdle();

		_swapChain.destroy();
		_swapChain.create(_config.EnableVsync);

		_currentEngineScene->recreate((uint32_t)width, (uint32_t)height);
		_currentEngineScene->updateAllDescriptorSets();
	}

	void Engine::createScene(EngineScene* scene)
	{
		ZoneScopedN("create engine scene");

		_currentEngineScene.reset(scene);
		_currentEngineScene->initialise(
			"camera_matrix_ubo",
			_device,
			_swapChain,
			_graphicsCommandPool,
			_transferCommandPool,
			_resourceManager,
			_materialSystem);
	}
	void Engine::destroyScene()
	{
		ZoneScopedN("destroy engine scene");

		_currentEngineScene->cleanup();
		_currentEngineScene.reset();
	}

}