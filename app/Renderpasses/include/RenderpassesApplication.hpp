#pragma once

#include <cstdint>
#include <helsinki/Renderer/Vulkan/VulkanInstance.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSurface.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanCommandPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSet.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSynchronisationContext.hpp>
#include <helsinki/Renderer/TempModel.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>

struct GLFWwindow;

namespace rp
{
	class RenderpassesApplication
	{
	public:
		RenderpassesApplication();
		void init(uint32_t width, uint32_t height, const char *title);
		void run();

		void notifyFramebufferResized() { framebufferResized = true; }

	private:
		void mainLoop();
		void cleanup();
		void draw();

		void initWindow(uint32_t width, uint32_t height, const char* title);
		void initVulkan(const char* title);
		void createUniformBuffers();
		void createCommandBuffers();
		void recreateSwapChain();
		void updateUniformBuffer(hl::VulkanUniformBuffer& uniformBuffer);
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		GLFWwindow* _window;
		hl::VulkanInstance _instance;
		hl::VulkanSurface _surface;
		hl::VulkanDevice _device;
		hl::VulkanCommandPool _commandPool;
		hl::VulkanCommandPool _oneTimeCommandPool;
		hl::VulkanSwapChain _swapChain;

		hl::VulkanRenderpassResources _renderpassResources;

		hl::TempModel _model;

		hl::VulkanSynchronisationContext _syncContext;

		std::vector<hl::VulkanUniformBuffer> _uniformBuffers;
		std::vector<hl::VulkanImage> _offscreenImages;

		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentFrame = 0;
		bool framebufferResized = false;
		bool _useMultiSampling = true; // TODO: Global (ish) variable for this rather than passing it around?
	};
}