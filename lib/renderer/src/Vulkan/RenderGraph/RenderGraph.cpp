#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/VulkanRenderGraphRenderpassResources.hpp>
#include <stdexcept>

namespace hl
{
	

	std::vector<VulkanRenderGraphRenderpassResources*> RenderGraph::create(
		const std::vector<hl::RenderpassInfo>& renderpassInfo, 
		VulkanDevice& device,
		uint32_t width,
		uint32_t height,
		uint32_t swapChainImageCount)
	{
		std::vector<VulkanRenderGraphRenderpassResources*> renderpasses;

		const auto lastName = renderpassInfo.back().name;

		for (const auto& ri : renderpassInfo)
		{
			auto isLastRenderpass = lastName == ri.name;

			auto imageCount = isLastRenderpass
				? swapChainImageCount :
				MAX_FRAMES_IN_FLIGHT;

			auto r = new VulkanRenderGraphRenderpassResources(
				ri.name,
				device, 
				imageCount);

			renderpasses.push_back(r);

			{
				for (const auto& res : ri.outputs)
				{
					// TODO: Foreach N where N is MAX_FRAMES_IN_FLIGHT or swapchain image count
					auto& attachment = r->addAttachment(res.name);
					attachment.type = res.type;
					attachment.format = extractFormat(res.format);

					VkImageUsageFlags usage = res.type == ResourceType::Color
						? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
						: VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

					if (ri.useMultiSampling)
					{
						usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
					}
					else
					{
						usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
					}

					for (uint32_t i = 0; i < imageCount; ++i)
					{
						if (res.type == ResourceType::Color)
						{
							auto image = new VulkanImage(device);
							attachment.images.push_back(image);

							image->create(
								width,
								height,
								1, // TODO
								ri.useMultiSampling
								? device._msaaSamples
								: VK_SAMPLE_COUNT_1_BIT,
								attachment.format,
								VK_IMAGE_TILING_OPTIMAL,
								usage,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

							image->createImageView(
								attachment.format,
								VK_IMAGE_ASPECT_COLOR_BIT,
								1);

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_image),
								VK_OBJECT_TYPE_IMAGE,
								(ri.name + "_" + res.name + std::string("_Image_") + std::to_string(0)).c_str());

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_imageMemory),
								VK_OBJECT_TYPE_DEVICE_MEMORY,
								(ri.name + "_" + res.name + std::string("_ImageMemory_") + std::to_string(i)).c_str());

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_imageView),
								VK_OBJECT_TYPE_IMAGE_VIEW,
								(ri.name + "_" + res.name + std::string("_ImageView_") + std::to_string(i)).c_str());

							// Dont create resolve images for the last one.  Needs to do swap chain magic
							if (ri.useMultiSampling && !isLastRenderpass) 
							{
								auto resolveImage = new VulkanImage(device);
								attachment.resolveImages.push_back(resolveImage);

								resolveImage->create(
									width,
									height,
									1,
									VK_SAMPLE_COUNT_1_BIT,
									attachment.format,
									VK_IMAGE_TILING_OPTIMAL,
									VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

								resolveImage->createImageView(
									attachment.format,
									VK_IMAGE_ASPECT_COLOR_BIT,
									1);

								// TODO: FOR EACH INDEX
								device.setDebugName(
									reinterpret_cast<uint64_t>(resolveImage->_image),
									VK_OBJECT_TYPE_IMAGE,
									(ri.name + "_" + res.name + std::string("_ResolveImage_") + std::to_string(i)).c_str());

								device.setDebugName(
									reinterpret_cast<uint64_t>(resolveImage->_imageMemory),
									VK_OBJECT_TYPE_DEVICE_MEMORY,
									(ri.name + "_" + res.name + std::string("_ResolveImageMemory_") + std::to_string(i)).c_str());

								device.setDebugName(
									reinterpret_cast<uint64_t>(resolveImage->_imageView),
									VK_OBJECT_TYPE_IMAGE_VIEW,
									(ri.name + "_" + res.name + std::string("_ResolveImageView_") + std::to_string(i)).c_str());

							}
						}
						else if (res.type == ResourceType::Depth)
						{
							auto image = new VulkanImage(device);
							attachment.images.push_back(image);

							image->create(
								width,
								height,
								1,
								ri.useMultiSampling
									? device._msaaSamples
									: VK_SAMPLE_COUNT_1_BIT,
								attachment.format,
								VK_IMAGE_TILING_OPTIMAL,
								usage,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

							image->createImageView(
								attachment.format,
								VK_IMAGE_ASPECT_DEPTH_BIT,
								1);

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_image),
								VK_OBJECT_TYPE_IMAGE,
								(ri.name + "_" + res.name + std::string("_Depth_") + std::to_string(i)).c_str());

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_imageMemory),
								VK_OBJECT_TYPE_DEVICE_MEMORY,
								(ri.name + "_" + res.name + std::string("_DepthMemory_") + std::to_string(i)).c_str());

							device.setDebugName(
								reinterpret_cast<uint64_t>(image->_imageView),
								VK_OBJECT_TYPE_IMAGE_VIEW,
								(ri.name + "_" + res.name + std::string("_DepthView_") + std::to_string(i)).c_str());

						}
						else
						{
							throw std::runtime_error("TODO NOT IMPLEMENTED");
						}
					}
				}

				// Renderpass
				{
					std::vector<VkAttachmentDescription> allAttachments;

					std::vector<VkAttachmentDescription> colorAttachments;
					std::vector<VkAttachmentReference> colorReferences;

					std::vector<VkAttachmentDescription> colorResolveAttachments;
					std::vector<VkAttachmentReference> colorResolveReferences;

					std::vector<VkAttachmentDescription> depthAttachments;
					std::vector<VkAttachmentReference> depthReferences;

					uint32_t referenceLayout = 0;

					for (const auto& ra : r->getAttachments())
					{
						VkAttachmentDescription description{};
						VkAttachmentReference reference{};

						description.format = ra.format;
						description.samples = ri.useMultiSampling
							? device._msaaSamples
							: VK_SAMPLE_COUNT_1_BIT;
						description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

						reference.attachment = referenceLayout;
						referenceLayout += 1;

						if (ra.type == ResourceType::Color)
						{
							description.finalLayout = ri.useMultiSampling
								? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
								: (isLastRenderpass
									? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
									: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

							reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

							colorAttachments.push_back(description);
							colorReferences.push_back(reference);

							allAttachments.push_back(description);

							if (ri.useMultiSampling)
							{
								VkAttachmentDescription resolveDescription{};
								VkAttachmentReference resolveReference{};

								resolveDescription.format = ra.format;
								resolveDescription.samples = VK_SAMPLE_COUNT_1_BIT;
								resolveDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
								resolveDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
								resolveDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
								resolveDescription.finalLayout = isLastRenderpass
									? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
									: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

								resolveReference.attachment = referenceLayout;
								resolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
								referenceLayout += 1;

								colorResolveAttachments.push_back(resolveDescription);
								colorResolveReferences.push_back(resolveReference);

								allAttachments.push_back(resolveDescription);
							}
						}
						else if (ra.type == ResourceType::Depth)
						{
							description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
							description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
							description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

							reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

							depthAttachments.push_back(description);
							depthReferences.push_back(reference);

							allAttachments.push_back(description);
						}
						else
						{
							throw std::runtime_error("NOT GOOD");
						}
					}

					if (depthReferences.size() > 1)
					{
						throw std::runtime_error("Can only have 1 depth attachment per subpass");
					}

					VkSubpassDescription subpass{};
					subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
					subpass.pColorAttachments = colorReferences.data();
					subpass.pDepthStencilAttachment = depthReferences.data();
					if (ri.useMultiSampling)
					{
						subpass.pResolveAttachments = colorResolveReferences.data();
					}
					else
					{
						subpass.pResolveAttachments = nullptr;
					}

					// TODO: Optimisation possibility
					VkSubpassDependency dependency{};
					dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
					dependency.dstSubpass = 0;
					dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

					// TODO: This will change if subpasses added
					VkRenderPassCreateInfo renderPassInfo{};
					renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
					renderPassInfo.attachmentCount = static_cast<uint32_t>(allAttachments.size());
					renderPassInfo.pAttachments = allAttachments.data();
					renderPassInfo.subpassCount = 1;
					renderPassInfo.pSubpasses = &subpass;
					renderPassInfo.dependencyCount = 1;
					renderPassInfo.pDependencies = &dependency;

					VkRenderPass vkRenderpass{ VK_NULL_HANDLE };

					if (vkCreateRenderPass(device._device, &renderPassInfo, nullptr, &vkRenderpass) != VK_SUCCESS)
					{
						throw std::runtime_error("failed to create render pass!");
					}

					r->addRenderpass(vkRenderpass);
				}
			}
		}

		return renderpasses;
	}

	void RenderGraph::destroy(std::vector<VulkanRenderGraphRenderpassResources*>& generatedRenderpassResources)
	{
		for (auto& r : generatedRenderpassResources)
		{
			r->destroy();
		}
	}

	VkFormat RenderGraph::extractFormat(const std::string& formatString)
	{
		if (formatString == "VK_FORMAT_R8G8B8A8_UNORM")
		{
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		else if (formatString == "VK_FORMAT_D32_SFLOAT")
		{
		
			return VK_FORMAT_D32_SFLOAT;
		}
		else if (formatString == "VK_FORMAT_B8G8R8A8_UNORM")
		{
			return VK_FORMAT_B8G8R8A8_UNORM;
		}
		else
		{
			throw std::runtime_error("Unhandled image format.");
		}
	}

}