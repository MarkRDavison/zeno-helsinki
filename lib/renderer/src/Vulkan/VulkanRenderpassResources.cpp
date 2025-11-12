#include <helsinki/Renderer/Vulkan/VulkanRenderpassResources.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <stdexcept>
#include <array>

// TODO: MAKE BETTER
#define ROOT_PATH(x) (std::string("F:/Workspace/Github/zeno-helsinki/app/Renderpasses") + std::string(x))

namespace hl
{


	VulkanRenderpassResources::VulkanRenderpassResources(
		const std::string& name,
		VulkanDevice& device
	) :
		_name(name),
		_device(device),
		_renderpass(device),
		_descriptorPool(device)
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

		_graphicPipelines.emplace_back(_device);
		auto& pipeline = _graphicPipelines.back();

		// Descriptor Set Layout
		{
			pipeline._descriptorSetLayout.create({
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
			pipeline._descriptorSet.create(_descriptorPool, pipeline._descriptorSetLayout);

			// TODO: Update at some point, but baby scene only needs it done once... so we do it in the main.cpp
		}

		// Graphics Pipeline
		{
			pipeline._graphicsPipeline.create(
				std::string(ROOT_PATH("/data/shaders/triangle.vert")),
				std::string(ROOT_PATH("/data/shaders/triangle.frag")),
				_renderpass,
				pipeline._descriptorSetLayout,
				multisSampling);
		}
	}
	void VulkanRenderpassResources::createPostProcess(
		VkFormat colorFormat,
		VkFormat depthFormat,
		VkExtent2D extent,
		bool multisSampling)
	{
		_multiSampling = multisSampling;
		_renderpassExtent = extent;

		// Color/Depth/resolve images
		{
			_colorFormat = colorFormat;
			_depthFormat = depthFormat;

			createImages(_colorFormat, _depthFormat, MAX_FRAMES_IN_FLIGHT, false);

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
			_renderpass.createBasicRenderpassWithFollowingRenderpass(
				multisSampling,
				_colorFormat);
		}

		// Framebuffer
		{
			createFramebuffers();
		}

		// Descriptor Pool
		{
			_descriptorPool.create();
		}

		_graphicPipelines.emplace_back(_device);
		auto& pipeline = _graphicPipelines.back();

		// Descriptor Set Layout
		{
			pipeline._descriptorSetLayout.create({
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
			pipeline._descriptorSet.create(_descriptorPool, pipeline._descriptorSetLayout);

			// Need to 'update' these, actually for static app need to initialize, but it requires the previous render pass to do so
		}
			
		// Graphics Pipeline
		{
			pipeline._graphicsPipeline.createPostProcess(
				std::string(ROOT_PATH("/data/shaders/post_process.vert")),
				std::string(ROOT_PATH("/data/shaders/post_process.frag")),
				_renderpass,
				pipeline._descriptorSetLayout,
				multisSampling);
		}
	}

	void VulkanRenderpassResources::createUi(VulkanSwapChain& swapChain, bool multisSampling)
	{
		_swapChain = &swapChain;
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

		{
			_graphicPipelines.emplace_back(_device);
			auto& pipeline = _graphicPipelines.back();

			// Descriptor Set Layout
			{
				pipeline._descriptorSetLayout.create({
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
				pipeline._descriptorSet.create(_descriptorPool, pipeline._descriptorSetLayout);

				// Need to 'update' these, actually for static app need to initialize, but it requires the previous render pass to do so
			}

			// Graphics Pipeline
			{
				pipeline._graphicsPipeline.createUi(
					std::string(ROOT_PATH("/data/shaders/fullscreen_sample.vert")),
					std::string(ROOT_PATH("/data/shaders/fullscreen_sample.frag")),
					_renderpass,
					pipeline._descriptorSetLayout,
					multisSampling,
					false);
			}
		}
		{
			_graphicPipelines.emplace_back(_device);
			auto& pipeline = _graphicPipelines.back();

			// Descriptor Set Layout
			{
			}

			// Descriptor Sets
			{
			}

			// Graphics Pipeline
			{
				pipeline._graphicsPipeline.createUi(
					std::string(ROOT_PATH("/data/shaders/ui.vert")),
					std::string(ROOT_PATH("/data/shaders/ui.frag")),
					_renderpass,
					pipeline._descriptorSetLayout,
					multisSampling,
					true);
			}
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
		for (auto& pipeline : _graphicPipelines)
		{
			pipeline.destroy();
		}

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
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(color._image), VK_OBJECT_TYPE_IMAGE, (_name + std::string("_ColorImage_") + std::to_string(i)).c_str());

				color.createImageView(
					imageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(color._imageView), VK_OBJECT_TYPE_IMAGE_VIEW, (_name + std::string("_ColorImageView_") + std::to_string(i)).c_str());

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
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(colorResolve._image), VK_OBJECT_TYPE_IMAGE, (_name + std::string("_ColorResolveImage_") + std::to_string(i)).c_str());

				colorResolve.createImageView(
					imageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(colorResolve._imageView), VK_OBJECT_TYPE_IMAGE_VIEW, (_name + std::string("_ColorResolveImageView_") + std::to_string(i)).c_str());
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
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(depth._image), VK_OBJECT_TYPE_IMAGE, (_name + std::string("_DepthImage_") + std::to_string(i)).c_str());

				depth.createImageView(
					depthFormat,
					VK_IMAGE_ASPECT_DEPTH_BIT,
					1);

				_device.setDebugName(reinterpret_cast<uint64_t>(depth._imageView), VK_OBJECT_TYPE_IMAGE_VIEW, (_name + std::string("_DepthImageView_") + std::to_string(i)).c_str());

			}
		}
	}
}