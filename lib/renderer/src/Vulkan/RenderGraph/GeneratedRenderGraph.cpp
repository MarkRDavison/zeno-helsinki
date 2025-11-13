#include <helsinki/Renderer/Vulkan/RenderGraph/GeneratedRenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{

	GeneratedRenderGraph::GeneratedRenderGraph(
        VulkanDevice& device,
        VulkanSwapChain& swapChain,
		std::vector<hl::RenderpassInfo> renderpasses,
		RenderResourcesSystem& renderResourcesSystem
	) :
        _device(device),
        _swapChain(swapChain),
        _renderResourcesSystem(renderResourcesSystem),
		_renderGraph(renderpasses)
	{
		_resources = hl::RenderGraph::create(
			renderResourcesSystem,
			renderpasses,
			device,
			swapChain._swapChainExtent.width,
			swapChain._swapChainExtent.height,
			swapChain._swapChainImageViews);

        updateAllOutputResources();
	}

    VkDescriptorSet GeneratedRenderGraph::getDescriptorSet(const std::string& renderpassName, const std::string& pipelineName, uint32_t frameNumber)
    {
        for (auto& r : _resources)
        {
            if (r->Name == renderpassName)
            {
                for (auto& p : r->getPipelines())
                {
                    if (p->Name == pipelineName)
                    {
                        return p->getDescriptorSet(frameNumber);
                    }
                }
            }
        }

        throw std::runtime_error("failed to find descriptor set");
    }


	std::vector<VulkanRenderGraphRenderpassResources*> GeneratedRenderGraph::getResources()
	{
		return _resources;
	}

	void GeneratedRenderGraph::destroy()
	{
		hl::RenderGraph::destroy(_resources);
	}

	void GeneratedRenderGraph::recreate(uint32_t width, uint32_t height)
	{
        size_t i = 0;
        for (auto& r : getResources())
        {
            const auto& info = _renderGraph[i];
            i++;

            auto isLastRenderpass = i == getResources().size();

            r->recreate(
                info,
                width,
                height,
                _swapChain._swapChainImageViews,
                (uint32_t)(isLastRenderpass ? _swapChain._swapChainImageViews.size() : MAX_FRAMES_IN_FLIGHT),
                isLastRenderpass);

            updateAllOutputResources();
        }
	}

	void GeneratedRenderGraph::updateAllDescriptorSets()
	{
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            for (auto& r : _renderGraph)
            {
                for (auto& p : r.pipelines)
                {
                    std::vector<VkWriteDescriptorSet> descriptorWrites;

                    size_t bindingCount = 0;

                    for (auto& ds : p.descriptorSets)
                    {
                        bindingCount += ds.bindings.size();
                    }

                    std::vector<VkDescriptorImageInfo> imageInfos;
                    std::vector<VkDescriptorBufferInfo> bufferInfos;

                    imageInfos.reserve(bindingCount);
                    bufferInfos.reserve(bindingCount);

                    for (auto& ds : p.descriptorSets)
                    {
                        for (auto& b : ds.bindings)
                        {
                            if (b.resource.has_value())
                            {
                                if (b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER")
                                {
                                    auto& ub = _renderResourcesSystem.getUniformBuffer(b.resource.value(), i);

                                    bufferInfos.push_back(VkDescriptorBufferInfo
                                        {
                                            .buffer = ub._buffer._buffer,
                                            .offset = 0,
                                            .range = ub._uboSize
                                        });

                                    descriptorWrites.emplace_back(VkWriteDescriptorSet
                                        {
                                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                            .dstSet = getDescriptorSet(r.name, p.name, i),
                                            .dstBinding = b.binding,
                                            .dstArrayElement = 0,
                                            .descriptorCount = 1,
                                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            .pBufferInfo = &bufferInfos.back()
                                        });
                                }
                                else if (b.type == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
                                {
                                    const auto& res = _renderResourcesSystem.getOffscreenImageOrTexture(b.resource.value(), i);

                                    imageInfos.push_back(VkDescriptorImageInfo
                                        {
                                            .sampler = res.second,
                                            .imageView = res.first,
                                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                        });


                                    descriptorWrites.emplace_back(VkWriteDescriptorSet
                                        {
                                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                            .dstSet = getDescriptorSet(r.name, p.name, i),
                                            .dstBinding = b.binding,
                                            .dstArrayElement = 0,
                                            .descriptorCount = 1,
                                            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            .pImageInfo = &imageInfos.back()
                                        });
                                }
                                else
                                {
                                    throw std::runtime_error("TODO: unhandled descriptor binding type");
                                }
                            }
                        }
                    }

                    vkUpdateDescriptorSets(
                        _device._device,
                        static_cast<uint32_t>(descriptorWrites.size()),
                        descriptorWrites.data(),
                        0,
                        nullptr);
                }
            }
        }
	}

    void GeneratedRenderGraph::updateAllOutputResources()
    {
        for (const auto& r : _renderGraph)
        {
            for (const auto& p : r.pipelines)
            {
                for (const auto& ds : p.descriptorSets)
                {
                    // Will be grouping at this level
                    for (const auto& b : ds.bindings)
                    {
                        // TODO: Look at inputs...
                        if (b.resource.has_value())
                        {
                            // std::cout << "Renderpass '" << r.name << "' - pipeline '" << p.name << "' - descriptor set '" << ds.name << "' - binding '" << b.resource.value() << "'" << std::endl;

                            for (const auto& grpr : _resources)
                            {
                                // Dont try and get an output from yourself as an input???
                                if (grpr->Name != r.name)
                                {
                                    for (auto& grpra : grpr->getAttachments())
                                    {
                                        if (grpra.name == b.resource.value())
                                        {
                                            // std::cout << " - Found it! Its the output from '" << grpr->Name << "'.'" << grpra.name << "'" << std::endl;

                                            std::vector<hl::VulkanImage*> offscreenImages;

                                            if (!grpra.resolveImages.empty())
                                            {
                                                for (auto i : grpra.resolveImages)
                                                {
                                                    offscreenImages.push_back(i);
                                                }
                                            }
                                            else
                                            {
                                                for (auto i : grpra.images)
                                                {
                                                    offscreenImages.push_back(i);
                                                }
                                            }

                                            if (grpra.sampler == VK_NULL_HANDLE)
                                            {
                                                // TODO: config
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

                                                CHECK_VK_RESULT(vkCreateSampler(_device._device, &samplerInfo, nullptr, &grpra.sampler));

                                                _device.setDebugName(
                                                    reinterpret_cast<uint64_t>(grpra.sampler),
                                                    VK_OBJECT_TYPE_SAMPLER,
                                                    (r.name + "_" + grpra.name + "_Sampler").c_str());
                                            }

                                            _renderResourcesSystem.registerOffscreenImage(
                                                b.resource.value(),
                                                offscreenImages,
                                                grpra.sampler);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}