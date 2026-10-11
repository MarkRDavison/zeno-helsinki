#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <helsinki/Renderer/Vulkan/VulkanGraphicsPipeline.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDescriptorSet.hpp>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT VulkanGraphicsPipelineResources
	{
	public:
		VulkanGraphicsPipelineResources(VulkanDevice& device);

		void destroy();

	public :// private: TODO: TO PRIVATE
		VulkanDevice& _device;
		VulkanDescriptorSetLayout _descriptorSetLayout;
		VulkanDescriptorSet _descriptorSet;
		VulkanGraphicsPipeline _graphicsPipeline;
	};

}