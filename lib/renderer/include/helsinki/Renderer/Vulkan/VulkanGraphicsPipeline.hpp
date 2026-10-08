#pragma once

#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanRenderpass.hpp>
#include <helsinki/Renderer/Vulkan/VulkanPipelineLayout.hpp>

namespace hl
{

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(VulkanDevice& device);

		void destroy();

		enum class ShaderCompileStage
		{
			Vertex,
			Fragment,
			Compute
		};

		static std::vector<uint32_t> readParseCompileShader(
			const std::string& shaderSource,
			bool vertex);
		static std::vector<uint32_t> readParseCompileShader(
			const std::string& shaderSource,
			ShaderCompileStage stage);

	public: // private: TODO: to private
		VulkanDevice& _device;
		VulkanPipelineLayout _pipelineLayout;

		VkPipeline _graphicsPipeline{VK_NULL_HANDLE};
	};

}