#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <stdexcept>
#include <array>

// TODO: MAKE BETTER
#define ROOT_PATH(x) (std::string("F:/Workspace/Github/zeno-helsinki/app/Renderpasses") + std::string(x))

namespace hl
{


	VulkanRenderpassResources::VulkanRenderpassResources(
		VulkanDevice& device
	) :
		_device(device),
		_renderpass(device),
		_descriptorPool(device),
		_descriptorSetLayout(device),
		_descriptorSet(device),
		_graphicsPipeline(device)
	{

	}

	void VulkanRenderpassResources::create(
		VkFormat colorFormat,
		VkFormat depthFormat,
		VkExtent2D extent,
		bool multisSampling)
	{
		_renderpassExtent = extent;
		_multiSampling = multisSampling;

		_colorFormat = colorFormat;
		_depthFormat = depthFormat;

		// Color/Depth/resolve images
		{
			createImages(_colorFormat, _depthFormat, MAX_FRAMES_IN_FLIGHT, _multiSampling);
		}

		// Renderpass
		{
			_renderpass.createBasicRenderpassWithFollowingRenderpass(
				multisSampling,
				colorFormat);
		}

		// Framebuffer
		{
			createFramebuffers();
		}

		// Descriptor Pool
		{
			_descriptorPool.create();
		}

		// Descriptor Set Layout
		{
			_descriptorSetLayout.create({
				VkDescriptorSetLayoutBinding
				{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.pImmutableSamplers = nullptr
				},
				VkDescriptorSetLayoutBinding
				{
					.binding = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = nullptr
				}
			});
		}

		// Descriptor Sets
		{
			_descriptorSet.create(_descriptorPool, _descriptorSetLayout);

			// TODO: Update at some point, but baby scene only needs it done once... so we do it in the main.cpp
		}

		// Graphics Pipeline
		{
			_graphicsPipeline.create(
				std::string(ROOT_PATH("/data/shaders/triangle.vert")),
				std::string(ROOT_PATH("/data/shaders/triangle.frag")),
				_renderpass,
				_descriptorSetLayout,
				multisSampling);
		}
	}
	void VulkanRenderpassResources::create(
		VulkanSwapChain& swapChain,
		bool multisSampling)
	{
		_swapChain = &swapChain; _multiSampling = multisSampling;
		_renderpassExtent = _swapChain->_swapChainExtent;

		// Color/Depth/resolve images
		{
			auto imageCount = _swapChain->_swapChainImages.size();
			_colorFormat = _swapChain->_swapChainImageFormat;
			_depthFormat = _swapChain->findDepthFormat(_device._physicalDevice);

			createImages(_colorFormat, _depthFormat, imageCount, false);

			{
				VkSamplerCreateInfo samplerInfo{};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.magFilter = VK_FILTER_LINEAR;
				samplerInfo.minFilter = VK_FILTER_LINEAR;
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.anisotropyEnable = VK_FALSE;
				samplerInfo.maxAnisotropy = 1.0f;
				samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerInfo.mipLodBias = 0.0f;
				samplerInfo.minLod = 0.0f;
				samplerInfo.maxLod = 0.0f;

				if (vkCreateSampler(_device._device, &samplerInfo, nullptr, &_outputSampler) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create sampler!");
				}
			}
		}

		// Renderpass
		{
			_renderpass.createBasicRenderpass(
				multisSampling,
				_swapChain->_swapChainImageFormat);
		}

		// Framebuffer
		{
			createFramebuffers();
		}

		// Descriptor Pool
		{
			_descriptorPool.create();
		}

		// Descriptor Set Layout
		{
			_descriptorSetLayout.create({
			   VkDescriptorSetLayoutBinding
			   {
				   .binding = 0,
				   .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				   .descriptorCount = 1,
				   .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				   .pImmutableSamplers = nullptr
			   }
			});
		}

		// Descriptor Sets
		{
			_descriptorSet.create(_descriptorPool, _descriptorSetLayout);

			// Need to 'update' these, actually for static app need to initialize, but it requires the previous render pass to do so
		}
			
		// Graphics Pipeline
		{
			_graphicsPipeline.createPostProcess(
				std::string(ROOT_PATH("/data/shaders/post_process.vert")),
				std::string(ROOT_PATH("/data/shaders/post_process.frag")),
				_renderpass,
				_descriptorSetLayout,
				multisSampling);
		}
	}

	void VulkanRenderpassResources::recreate(VkExtent2D extent)
	{
		_renderpassExtent = extent;

		if (_swapChain != nullptr)
		{
			_swapChain->destroy();
			_swapChain->create();
		}

		{	// Destroy resources
			for (auto& colorImage : _colorImages)
			{
				colorImage.destroy();
			}
			_colorImages.clear();

			for (auto& colorResolveImage : _colorResolveImages)
			{
				colorResolveImage.destroy();
			}
			_colorResolveImages.clear();

			for (auto& depthImage : _depthImages)
			{
				depthImage.destroy();
			}
			_depthImages.clear();

			for (auto& frameBuffer : _framebuffers)
			{
				frameBuffer.destroy();
			}
			_framebuffers.clear();

		}
		{	// Create resources

			size_t imageCount = 0;
			VkFormat imageFormat = VK_FORMAT_UNDEFINED;
			VkFormat depthFormat = VK_FORMAT_UNDEFINED;
			bool createResolveColors = false;

			if (_swapChain != nullptr)
			{
				imageCount = _swapChain->_swapChainImages.size();
				imageFormat = _colorFormat;
				depthFormat = _depthFormat;
				createResolveColors = false;
			}
			else
			{
				imageCount = MAX_FRAMES_IN_FLIGHT;
				imageFormat = _colorFormat;
				depthFormat = _depthFormat;
				createResolveColors = _multiSampling;
			}

			createImages(imageFormat, depthFormat, imageCount, createResolveColors);
			createFramebuffers();
		}
	}

	void VulkanRenderpassResources::destroy()
	{
		_graphicsPipeline.destroy();


		_descriptorSetLayout.destroy();

		_descriptorPool.destroy();

		for (auto& frameBuffer : _framebuffers)
		{
			frameBuffer.destroy();
		}

		_renderpass.destroy();

		if (_outputSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(_device._device, _outputSampler, nullptr);
		}

		for (auto& colorImage : _colorImages)
		{
			colorImage.destroy();
		}

		for (auto& colorResolveImage : _colorResolveImages)
		{
			colorResolveImage.destroy();
		}

		for (auto& depthImage : _depthImages)
		{
			depthImage.destroy();
		}
	}

	void VulkanRenderpassResources::recordCommandBuffer(
		VkCommandBuffer commandBuffer, 
		uint32_t imageIndex,
		std::function<void()> operation)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderpass._renderPass;
		renderPassInfo.framebuffer = _framebuffers[imageIndex]._framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _renderpassExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// TODO: Do I need to always clear?
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline._graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)_renderpassExtent.width;
		viewport.height = (float)_renderpassExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = _renderpassExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		{
			operation();
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanRenderpassResources::createFramebuffers()
	{
		size_t framebufferCount = 0;

		// TODO: CHANGE TO PARAMETER???
		if (_swapChain != nullptr)
		{
			framebufferCount = _swapChain->_swapChainImages.size();
		}
		else
		{
			framebufferCount = MAX_FRAMES_IN_FLIGHT;
		}

		for (uint32_t i = 0; i < framebufferCount; ++i)
		{
			_framebuffers.emplace_back(_device);
			auto& frameBuffer = _framebuffers.back();

			std::vector<VkImageView> attachments;
			if (_swapChain != nullptr)
			{
				if (_multiSampling)
				{
					attachments = {
						_colorImages[i]._imageView,
						_depthImages[i]._imageView,
						_swapChain->_swapChainImageViews[i]
					};
				}
				else
				{
					attachments = {
						_swapChain->_swapChainImageViews[i],
						_depthImages[i]._imageView
					};
				}
			}
			else
			{
				if (_multiSampling)
				{
					attachments = {
						_colorImages[i]._imageView,
						_depthImages[i]._imageView,
						_colorResolveImages[i]._imageView
					};
				}
				else
				{
					attachments = {
						_colorImages[i]._imageView,
						_depthImages[i]._imageView
					};
				}
			}

			frameBuffer.create(
				_renderpass,
				attachments,
				_renderpassExtent.width,
				_renderpassExtent.height);
		}
	}

	void VulkanRenderpassResources::createImages(VkFormat imageFormat, VkFormat depthFormat, size_t imageCount, bool createResolveImages)
	{

		for (uint32_t i = 0; i < imageCount; ++i)
		{
			{
				VkImageUsageFlags usage =
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

				if (_multiSampling)
				{
					usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
				}
				else
				{
					usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
				}

				_colorImages.emplace_back(_device);
				auto& color = _colorImages.back();

				color.create(
					_renderpassExtent.width,
					_renderpassExtent.height,
					1,
					_multiSampling
						? _device._msaaSamples
						: VK_SAMPLE_COUNT_1_BIT,
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					usage,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				color.createImageView(
					imageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1);
			}

			if (createResolveImages)
			{
				_colorResolveImages.emplace_back(_device);
				auto& colorResolve = _colorResolveImages.back();

				colorResolve.create(
					_renderpassExtent.width,
					_renderpassExtent.height,
					1,
					VK_SAMPLE_COUNT_1_BIT,
					imageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				colorResolve.createImageView(
					imageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1);
			}

			{
				_depthImages.emplace_back(_device);
				auto& depth = _depthImages.back();

				depth.create(
					_renderpassExtent.width,
					_renderpassExtent.height,
					1,
					_multiSampling
						? _device._msaaSamples
						: VK_SAMPLE_COUNT_1_BIT,
					depthFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				depth.createImageView(
					depthFormat,
					VK_IMAGE_ASPECT_DEPTH_BIT,
					1);
			}
		}
	}
}