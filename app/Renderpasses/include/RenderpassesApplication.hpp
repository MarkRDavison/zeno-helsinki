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
#include <helsinki/Renderer/Vulkan/VulkanSynchronisationContext.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/GeneratedRenderGraph.hpp>
#include <helsinki/Renderer/Resource/TextureResource.hpp>
#include <helsinki/Renderer/Resource/CubemapTextureResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/Resource/BasicModelResource.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/Events/EventBus.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include "RenderpassesConfig.hpp"

struct GLFWwindow;

namespace rp
{
	class RenderpassesApplication : public hl::EventListener
	{
	public:
		RenderpassesApplication(hl::EventBus& eventBus);
		~RenderpassesApplication();
		void init(RenderpassConfig config);
		void run();
		void notifyFramebufferResized(int width, int height);
		void sendEvent(const hl::Event& event);

		void OnEvent(const hl::Event& event) override;

	private:
		void mainLoop();
		void cleanup();
		void update(float delta);
		void draw();

		void initWindow(uint32_t width, uint32_t height, const char* title);
		void initVulkan(const char* title);
		void createCommandBuffers();
		void recreateSwapChain();
		void updateUniformBuffer(hl::VulkanUniformBuffer& uniformBuffer);
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void renderPipelineDraw(VkCommandBuffer commandBuffer, hl::VulkanRenderGraphPipelineResources* pipeline);

	private:
		hl::EventBus& _eventBus;
		GLFWwindow* _window;
		hl::VulkanInstance _instance;
		hl::VulkanSurface _surface;
		hl::VulkanDevice _device;
		hl::VulkanCommandPool _commandPool;
		hl::VulkanCommandPool _oneTimeCommandPool;
		hl::VulkanSwapChain _swapChain;

		hl::VulkanSynchronisationContext _syncContext;

		std::vector<VkCommandBuffer> _perFrameCommandBuffers;

		hl::GeneratedRenderGraph* _renderGraph{ nullptr };

		hl::ResourceManager _resourceManager;

		hl::ResourceHandle<hl::UniformBufferResource> _modelMatrixHandle;
		hl::ResourceHandle<hl::BasicModelResource> _modelHandle;
		hl::Camera* _camera{ nullptr };

		RenderpassConfig _config;

		uint32_t currentFrame = 0;
		bool framebufferResized = false;
	};
}