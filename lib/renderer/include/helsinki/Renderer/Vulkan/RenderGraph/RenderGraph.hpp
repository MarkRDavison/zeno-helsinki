#pragma once

#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <unordered_map>
#include <optional>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

namespace hl
{
    class VulkanRenderGraphRenderpassResources;
    class ResourceManager;

    struct Node
    {
        enum class Visit { NotVisited, Visiting, Done };

        std::string name;
        std::vector<std::string> next;
        std::vector<std::string> prev;

        uint32_t layer{ std::numeric_limits<uint32_t>::max() };
        Visit state{ Visit::NotVisited };
    };

    enum class ResourceType
    {
        Color,
        Depth
    };

    enum class VertexAttributeFormat
    {
        Float,
        Vec2,
        Vec3,
        Vec4
    };

    struct ResourceInfo
    {
        std::string name;
        ResourceType type;
        std::string format; // e.g., "VK_FORMAT_R8G8B8A8_UNORM"
        std::optional<std::string> source; // optional, used if this resource comes from a previous pass
        std::optional<VkClearValue> clear;
        bool useMultiSampling{ false };
    };

    enum class DescriptorUpdateFrequency
    {
        Static,
        PerFrame
    };

    inline bool shouldWriteDescriptorBinding(
        DescriptorUpdateFrequency frequency,
        bool staticDescriptorsWritten)
    {
        return !(frequency == DescriptorUpdateFrequency::Static && staticDescriptorsWritten);
    }

    struct DescriptorBinding
    {
        uint32_t binding;
        std::string type;           // e.g., "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER", "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"
        std::string stage;          // "VERTEX", "FRAGMENT"
        std::optional<std::string> resource; // name of the resource bound to this descriptor
        uint32_t count{ 1 };
        DescriptorUpdateFrequency updateFrequency{ DescriptorUpdateFrequency::PerFrame };
        bool partiallyBound{ false };
        bool updateAfterBind{ false };
    };

    inline uint32_t descriptorArrayWriteCount(
        uint32_t bindingCount,
        uint32_t childCount,
        bool partiallyBound)
    {
        if (partiallyBound)
        {
            return childCount;
        }

        return bindingCount;
    }

    inline VkDescriptorBindingFlags descriptorBindingVkFlags(const DescriptorBinding& binding)
    {
        VkDescriptorBindingFlags flags = 0;
        if (binding.partiallyBound)
        {
            flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        }
        if (binding.updateAfterBind)
        {
            flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        }
        return flags;
    }

    struct DescriptorSetInfo
    {
        std::string name;
        std::vector<DescriptorBinding> bindings;
    };

    inline bool descriptorBindingsLayoutEqual(const DescriptorBinding& a, const DescriptorBinding& b)
    {
        return a.binding == b.binding
            && a.type == b.type
            && a.stage == b.stage
            && a.count == b.count
            && a.partiallyBound == b.partiallyBound
            && a.updateAfterBind == b.updateAfterBind;
    }

    inline bool descriptorSetLayoutsCompatible(
        const std::vector<DescriptorSetInfo>& a,
        const std::vector<DescriptorSetInfo>& b)
    {
        if (a.size() != b.size())
        {
            return false;
        }

        for (size_t setIndex = 0; setIndex < a.size(); ++setIndex)
        {
            const auto& left = a[setIndex].bindings;
            const auto& right = b[setIndex].bindings;
            if (left.size() != right.size())
            {
                return false;
            }

            for (size_t bindingIndex = 0; bindingIndex < left.size(); ++bindingIndex)
            {
                if (!descriptorBindingsLayoutEqual(left[bindingIndex], right[bindingIndex]))
                {
                    return false;
                }
            }
        }

        return true;
    }

    struct VertexAttributeInfo
    {
        std::string name;
        VertexAttributeFormat format;
        uint32_t location;
        uint32_t offset;
    };

    struct VertexInputInfo
    {
        std::vector<VertexAttributeInfo> attributes;
        uint32_t stride;
    };

    struct DepthState
    {
        bool testEnable = true;
        bool writeEnable = true;
        VkCompareOp compareOp = VK_COMPARE_OP_LESS;
    };

    struct RasterState
    {
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }; 
    
    enum class ViewportMode
    {
        Fill,
        FixedAspect, 
        FixedResolution, 
        Custom
    };
    
    struct ViewportInfo
    {
        ViewportMode mode{ ViewportMode::Fill };
        uint32_t width;
        uint32_t height;
    };

    enum class PipelineBindPoint
    {
        Graphics,
        Compute
    };

    struct PipelineInfo
    {
        std::string name;
        std::string shaderVert;
        std::string shaderFrag;
        std::string shaderComp;
        PipelineBindPoint bindPoint{ PipelineBindPoint::Graphics };
        std::vector<DescriptorSetInfo> descriptorSets;
        std::optional<VertexInputInfo> vertexInputInfo;
        DepthState depthState;
        RasterState rasterState;
        bool enableBlending = false;
        uint32_t pushConstantSize;
        ViewportInfo viewport;
    };

    struct RenderpassInfo
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<ResourceInfo> outputs;
        std::vector<std::string> bufferOutputs;
        std::vector<std::vector<PipelineInfo>> pipelineGroups;
        VkExtent2D extent{};
    };

    inline bool passUsesMultiSampling(const RenderpassInfo& pass)
    {
        if (pass.outputs.empty())
        {
            return false;
        }

        const bool first = pass.outputs.front().useMultiSampling;
        for (const auto& output : pass.outputs)
        {
            if (output.useMultiSampling != first)
            {
                throw std::runtime_error(
                    "Pass '" + pass.name + "' has mixed useMultiSampling on outputs");
            }
        }

        return first;
    }

    inline bool passIsCompute(const RenderpassInfo& pass)
    {
        std::optional<PipelineBindPoint> bindPoint;
        for (const auto& group : pass.pipelineGroups)
        {
            for (const auto& pipeline : group)
            {
                if (bindPoint.has_value() && *bindPoint != pipeline.bindPoint)
                {
                    throw std::runtime_error(
                        "Pass '" + pass.name + "' mixes graphics and compute pipelines");
                }
                bindPoint = pipeline.bindPoint;

                if (pipeline.bindPoint == PipelineBindPoint::Graphics && !pipeline.shaderComp.empty())
                {
                    throw std::runtime_error(
                        "Pass '" + pass.name + "' graphics pipeline '" + pipeline.name
                        + "' must not set shaderComp");
                }
                if (pipeline.bindPoint == PipelineBindPoint::Compute
                    && (!pipeline.shaderVert.empty() || !pipeline.shaderFrag.empty()))
                {
                    throw std::runtime_error(
                        "Pass '" + pass.name + "' compute pipeline '" + pipeline.name
                        + "' must not set shaderVert/shaderFrag");
                }
            }
        }

        if (!bindPoint.has_value() || *bindPoint == PipelineBindPoint::Graphics)
        {
            return false;
        }

        if (!pass.outputs.empty())
        {
            throw std::runtime_error(
                "Compute pass '" + pass.name + "' must not have color/depth outputs");
        }

        return true;
    }

    struct RenderpassAttachment
    {
        std::string name;
        ResourceType type{ ResourceType::Color };  // TODO: DEFAULT VALUE?
        VkFormat format{ VK_FORMAT_UNDEFINED };
        std::vector<VulkanImage*> images;
        std::vector<VulkanImage*> resolveImages; // TODO: This might be the swapchain image somehow
        VkSampler sampler{ VK_NULL_HANDLE }; // TODO: This is populated when a subsequent renderpass needs it
    };

    enum class GraphImageBarrierKind
    {
        UndefinedToColorAttachment,
        UndefinedToDepthAttachment,
        ColorAttachmentToSampled,
        ColorAttachmentToPresent
    };

    struct GraphImageBarrierEdge
    {
        std::string passName;
        std::string resourceName;
        GraphImageBarrierKind kind;
    };

    class RenderGraph
    {
        RenderGraph() = delete;
    public:
        static std::vector<VulkanRenderGraphRenderpassResources*> create(
            const std::vector<hl::RenderpassInfo>& renderpassInfo, 
            VulkanDevice& device,
            uint32_t width,
            uint32_t height,
            const std::vector<VkImageView>& swapChainImageViews,
            VkFormat swapChainFormat,
            ResourceManager& resourceManager);

        static void destroy(std::vector<VulkanRenderGraphRenderpassResources*>& generatedRenderpassResources);

        static void createImages(
            VulkanDevice& device,
            VulkanRenderGraphRenderpassResources* resources,
            const RenderpassInfo& info,
            uint32_t width,
            uint32_t height,
            uint32_t imageCount,
            bool isLastRenderpass);

        static VkFormat extractFormat(const std::string& formatString);
        static VkDescriptorType extractDescriptorType(const std::string& descriptorTypeString);
        static uint32_t extractStage(const std::string& stage);
        static VkFormat extractVertexAttributeFormat(VertexAttributeFormat format);

        static std::unordered_map<std::string, Node> generateDAG(const std::vector<hl::RenderpassInfo>& renderpassInfo);
        static std::vector<GraphImageBarrierEdge> generateImageBarrierEdges(const std::vector<hl::RenderpassInfo>& renderpassInfo);
    };

}