#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanFramebuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSet.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <functional>

namespace hl
{
	class VulkanRenderpassResources
	{
	public:
		VulkanRenderpassResources(VulkanDevice& device);

		void create(
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkExtent2D extent,
			bool multisSampling);
		void create(
			VulkanSwapChain& swapChain,
			bool multisSampling);
		void recreate(VkExtent2D extent);
		void destroy();

		void recordCommandBuffer(
			VkCommandBuffer commandBuffer, 
			uint32_t imageIndex,
			std::function<void()> operation);

	private:
		void createFramebuffers();
		void createImages();

	public: // private: TODO: TO PRIVATE
		VulkanDevice& _device;
		VulkanRenderpass _renderpass;
		VulkanDescriptorPool _descriptorPool;
		VulkanDescriptorSetLayout _descriptorSetLayout;
		VulkanDescriptorSet _descriptorSet;
		VulkanGraphicsPipeline _graphicsPipeline;

		std::vector<VulkanFramebuffer> _framebuffers;
		std::vector<VulkanImage> _colorImages;
		std::vector<VulkanImage> _depthImages;

		VkExtent2D _renderpassExtent;

		VulkanSwapChain* _swapChain{ nullptr };
		bool _multiSampling{ false };
	};
}