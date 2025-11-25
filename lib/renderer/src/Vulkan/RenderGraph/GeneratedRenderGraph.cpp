#include <helsinki/Renderer/Vulkan/RenderGraph/GeneratedRenderGraph.hpp>
#include <helsinki/Renderer/Vulkan/VulkanUniformBuffer.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/Renderer/Resource/UniformBufferResource.hpp>
#include <helsinki/Renderer/Resource/OffscreenImageResource.hpp>
#include <stdexcept>
#include <iostream>

namespace hl
{

	GeneratedRenderGraph::GeneratedRenderGraph(
        VulkanDevice& device,
        VulkanSwapChain& swapChain,
		std::vector<hl::RenderpassInfo> renderpasses,
        ResourceManager& resourceManager
	) :
        _device(device),
        _swapChain(swapChain),
		_renderGraph(renderpasses),
        _dag(hl::RenderGraph::generateDAG(renderpasses)),
        _resourceManager(resourceManager)
	{
		_resources = hl::RenderGraph::create(
			renderpasses,
			device,
			swapChain._swapChainExtent.width,
			swapChain._swapChainExtent.height,
			swapChain._swapChainImageViews);

        for (uint32_t layer = 0;; ++layer)
        {
            bool found = false;
            for (auto& [_, node] : _dag)
            {
                if (node.layer == layer)
                {
                    _sortedNodes.push_back(&node);
                    found = true;
                }
            }

            if (!found) 
            { 
                break; 
            }
        }

        for (auto& [name, node] : _dag)
        {
            _nodesByLayer[node.layer].push_back(name);
        }
	}

    VkDescriptorSet GeneratedRenderGraph::getDescriptorSet(const std::string& renderpassName, const std::string& pipelineName, uint32_t frameNumber)
    {
        for (auto& r : _resources)
        {
            if (r->Name == renderpassName)
            {
                for (auto& pg : r->getPipelineGroups())
                {
                    for (auto& p : pg)
                    {
                        if (p->Name == pipelineName)
                        {
                            return p->getDescriptorSet(frameNumber);
                        }
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
    VulkanRenderGraphRenderpassResources* GeneratedRenderGraph::getRenderpassByName(const std::string& name)
    {
        for (auto& r : _resources)
        {
            if (r->Name == name)
            {
                return r;
            }
        }

        throw std::runtime_error("Could not find renderpass with given name");
    }

    std::vector<std::string> GeneratedRenderGraph::getSortedNodesByName() const
    {
        std::vector<std::string> names;

        for (const auto& s : _sortedNodes)
        {
            names.push_back(s->name);
        }

        return names;
    }

    const std::vector<std::string>& GeneratedRenderGraph::getSortedNodesByNameForLayer(uint32_t layer) const
    {
        return _nodesByLayer.at(layer);
    }

    uint32_t GeneratedRenderGraph::getNumberLayers() const
    {
        return (uint32_t)_nodesByLayer.size();
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
                for (auto& pg : r.pipelineGroups)
                {
                    for (auto& p : pg)
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
                                    if (b.type == "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER" ||
                                        b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER" ||
                                        b.type == "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC")
                                    {
                                        auto& ub = _resourceManager
                                            .GetResource<UniformBufferResource>(
                                                b.resource.value())
                                            ->getUniformBuffer(i);

                                        bufferInfos.push_back(VkDescriptorBufferInfo
                                            {
                                                .buffer = ub._buffer._buffer,
                                                .offset = 0,
                                                .range = ub._size / ub._multiple
                                            });

                                        descriptorWrites.emplace_back(VkWriteDescriptorSet
                                            {
                                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                .dstSet = getDescriptorSet(r.name, p.name, i),
                                                .dstBinding = b.binding,
                                                .dstArrayElement = 0,
                                                .descriptorCount = 1,
                                                .descriptorType = RenderGraph::extractDescriptorType(b.type),
                                                .pBufferInfo = &bufferInfos.back()
                                            });
                                    }
                                    else if (b.type == "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER")
                                    {
                                        auto info = _resourceManager
                                            .GetResource<ImageSamplerResource>(
                                                b.resource.value())
                                            ->getDescriptorInfo(i);

                                        imageInfos.push_back(VkDescriptorImageInfo
                                            {
                                                .sampler = info.first,
                                                .imageView = info.second,
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
	}

    void GeneratedRenderGraph::updateAllOutputResources()
    {
        for (const auto& r : _renderGraph)
        {
            for (const auto& pg : r.pipelineGroups)
            {
                for (const auto& p : pg)
                {
                    for (const auto& ds : p.descriptorSets)
                    {
                        for (const auto& b : ds.bindings)
                        {
                            if (b.resource.has_value())
                            {
                                for (const auto& grpr : _resources)
                                {
                                    if (grpr->Name != r.name)
                                    {
                                        for (auto& grpra : grpr->getAttachments())
                                        {
                                            if (grpra.name == b.resource.value())
                                            {
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

                                                // Create load if does not exist?
                                                if (!_resourceManager.HasResource<ImageSamplerResource>(b.resource.value()))
                                                {
                                                    _resourceManager.LoadAs<OffscreenImageResource, ImageSamplerResource>(b.resource.value());
                                                }

                                                auto samplerResource = _resourceManager.GetResourceAs<OffscreenImageResource, ImageSamplerResource>(b.resource.value());

                                                std::vector<VkImageView> imageViews;

                                                for (auto& image : offscreenImages)
                                                {
                                                    imageViews.push_back(image->_imageView);
                                                }

                                                samplerResource->set(grpra.sampler, imageViews);
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
}