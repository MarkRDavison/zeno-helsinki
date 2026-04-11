#pragma once

#include <helsinki/Engine/Scene/Scene.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphPipelineResources.hpp>

namespace hl
{
	struct PipelineDrawData
	{
		uint32_t currentFrame;
		VkCommandBuffer commandBuffer;
		hl::VulkanRenderGraphPipelineResources* pipeline;
		Scene* scene;
	};
}