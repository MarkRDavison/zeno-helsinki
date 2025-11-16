#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanFramebuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSet.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipelineResources.hpp>
#include <functional>

namespace hl
{
	class VulkanRenderpassResources
	{
	public:
		VulkanRenderpassResources(const std::string& name, VulkanDevice& device);

		void create(
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkExtent2D extent,
			bool multisSampling);
		void createPostProcess(
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkExtent2D extent,
			bool multisSampling);
		void createUi(VulkanSwapChain& swapChain, bool multisSampling);
		void recreate(VkExtent2D extent, bool useVsync);
		void destroy();

	private:
		void createFramebuffers();
		void createImages(VkFormat imageFormat, VkFormat depthFormat, size_t imageCount, bool createResolveImages);

	public: // private: TODO: TO PRIVATE
		const std::string _name;
		VulkanDevice& _device;
		VulkanRenderpass _renderpass;
		VulkanDescriptorPool _descriptorPool;

		std::vector<VulkanGraphicsPipelineResources> _graphicPipelines;

		std::vector<VulkanFramebuffer> _framebuffers;
		std::vector<VulkanImage> _colorImages;
		std::vector<VulkanImage> _depthImages;
		std::vector<VulkanImage> _colorResolveImages;

		VkSampler _outputSampler { VK_NULL_HANDLE };

		VkFormat _colorFormat{ VK_FORMAT_UNDEFINED };
		VkFormat _depthFormat{ VK_FORMAT_UNDEFINED };

		VkExtent2D _renderpassExtent{ 0,0 };

		VulkanSwapChain* _swapChain{ nullptr };
		bool _multiSampling{ false };
	};
}