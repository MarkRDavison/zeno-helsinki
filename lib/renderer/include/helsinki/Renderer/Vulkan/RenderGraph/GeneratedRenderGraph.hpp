#pragma once

#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderResourcesSystem.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>

namespace hl
{

	class GeneratedRenderGraph
	{
	public:
		GeneratedRenderGraph(
			VulkanDevice& device,
			VulkanSwapChain& swapChain,
			std::vector<hl::RenderpassInfo> renderpasses,
			RenderResourcesSystem& renderResourcesSystem);

		VkDescriptorSet getDescriptorSet(const std::string& renderpassName, const std::string& pipelineName, uint32_t frameNumber);

		std::vector<std::vector<VulkanRenderGraphRenderpassResources*>> getResources();

		void destroy();
		void recreate(uint32_t width, uint32_t height);

		void updateAllDescriptorSets();
		void updateAllOutputResources();

	private:
		VulkanDevice& _device;
		VulkanSwapChain& _swapChain;
		const std::vector<hl::RenderpassInfo> _renderGraph;
		RenderResourcesSystem& _renderResourcesSystem;
		std::vector<std::vector<VulkanRenderGraphRenderpassResources*>> _resources;
	};

}