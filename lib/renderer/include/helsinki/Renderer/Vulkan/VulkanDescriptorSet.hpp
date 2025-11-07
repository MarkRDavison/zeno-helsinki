#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanTexture.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorPool.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSetLayout.hpp>

namespace hl
{
	class VulkanDescriptorSet 
	{
	public:
		VulkanDescriptorSet(VulkanDevice& device);

		void create(VulkanDescriptorPool& pool, VulkanDescriptorSetLayout& layout);

		void update(size_t index, VulkanUniformBuffer& uniform, VulkanTexture& texture);

		void updatePostProcess(size_t index, VulkanImage& offscreenImage, VkSampler& offscreenSampler);

	public: // private: TODO: To private
		VulkanDevice& _device;
		std::vector<VkDescriptorSet> _descriptorSets;
	};
}