#pragma once

#include <helsinki/Renderer/Vulkan/RenderGraph/GeneratedRenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/FrameResources.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/System/Utils/NonCopyable.hpp>
#include <helsinki/System/Infrastructure/Camera.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>

namespace hl
{

	class EngineScene : NonCopyable
	{
	public:
		EngineScene();
		~EngineScene();

		void initialise(
			const std::string& cameraMatrixResourceId,
			VulkanDevice& device,
			VulkanSwapChain& swapChain,
			VulkanCommandPool& graphicsCommandPool,
			VulkanCommandPool& transferCommandPool,
			ResourceManager& resourceManager,
			MaterialSystem& materialSystem,
			const std::vector<RenderpassInfo>& renderpassInfo);
		virtual void initialise(
			const std::string& cameraMatrixResourceId,
			VulkanDevice& device,
			VulkanSwapChain& swapChain,
			VulkanCommandPool& graphicsCommandPool,
			hl::VulkanCommandPool& transferCommandPool,
			ResourceManager& resourceManager,
			MaterialSystem& materialSystem);
		void cleanup();

		virtual void update(uint32_t currentFrame, float delta);
		VkCommandBuffer draw(uint32_t currentFrame, uint32_t imageIndex);

		void recreate(uint32_t width, uint32_t height);
		void updateAllDescriptorSets();
		void updateAllOutputResources();

	private: // TODO: TEMP

		void updateCameraUniformBuffer(VulkanUniformBuffer& uniformBuffer);
		void renderPipelineDraw(VkCommandBuffer commandBuffer, hl::VulkanRenderGraphPipelineResources* pipeline, uint32_t currentFrame);

	protected:
		Scene _scene;
		VulkanDevice* _device;
		VulkanSwapChain* _swapChain;
		ResourceManager* _resourceManager;
		MaterialSystem* _materialSystem;

	private:
		std::vector<FrameResources> _frameResources;
		GeneratedRenderGraph* _renderGraph{ nullptr };
		ResourceHandle<UniformBufferResource> _cameraMatrixPushConstantHandle;
		Camera* _camera; // TODO: We may want multiple cameras
	};

}