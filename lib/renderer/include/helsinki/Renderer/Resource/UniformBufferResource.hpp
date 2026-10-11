#pragma once

#include <helsinki/Renderer/Export.hpp>

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

namespace hl
{

	class HELSINKI_RENDERER_EXPORT UniformBufferResource : public Resource
	{
	public:
		explicit UniformBufferResource(
			const std::string& id, 
			ResourceContext& context,
			VkDeviceSize size,
			uint32_t count,
			uint32_t multiple);

		bool Load() override;
		void Unload() override;

		VulkanUniformBuffer& getUniformBuffer(uint32_t frame);

	private:
		VulkanDevice& _device;
		VkDeviceSize _size;
		uint32_t _count;
		uint32_t _multiple;

		std::vector<VulkanUniformBuffer> _buffers;
	};

}