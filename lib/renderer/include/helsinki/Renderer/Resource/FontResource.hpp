#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>

namespace hl
{

	class FontResource : public Resource
	{
	public:
		explicit FontResource(
			const std::string& id,
			ResourceContext& context);

		bool Load() override;
		void Unload() override;



	private:
		VulkanDevice& _device;
		VulkanCommandPool& _commandPool;
		ResourceManager& _resourceManager;
		const std::string _rootPath;
	};

}