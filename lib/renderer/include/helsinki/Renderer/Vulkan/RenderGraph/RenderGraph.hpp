#pragma once

#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderResourcesSystem.hpp>
#include <string>
#include <vector>
#include <optional>

namespace hl
{
    class VulkanRenderGraphRenderpassResources;

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
    };

    struct DescriptorBinding
    {
        uint32_t binding;
        std::string type;           // e.g., "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER", "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"
        std::string stage;          // "VERTEX", "FRAGMENT"
        std::optional<std::string> resource; // name of the resource bound to this descriptor
    };

    struct DescriptorSetInfo
    {
        std::string name;
        std::vector<DescriptorBinding> bindings;
    };

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

    struct PipelineInfo
    {
        std::string name;
        std::string shaderVert;
        std::string shaderFrag;
        std::vector<DescriptorSetInfo> descriptorSets;
        std::optional<VertexInputInfo> vertexInputInfo;
        bool enableBlending = false;
    };

    struct RenderpassInfo
    {
        std::string name;
        bool useMultiSampling;// TODO: Move this to the outputs ResourceInfo type
        std::vector<std::string> inputs;           // names of input resources
        std::vector<ResourceInfo> outputs;         // resources this pass writes
        std::vector<PipelineInfo> pipelines;       // pipelines in this pass
    };

    struct RenderpassAttachment
    {
        std::string name;
        ResourceType type{ ResourceType::Color };  // TODO: DEFAULT VALUE?
        VkFormat format{ VK_FORMAT_UNDEFINED };
        std::vector<VulkanImage*> images;
        std::vector<VulkanImage*> resolveImages; // TODO: This might be the swapchain image somehow
        VkSampler sampler{ VK_NULL_HANDLE }; // TODO: This is populated when a subsequent renderpass needs it
    };

    class RenderGraph
    {
        RenderGraph() = delete;
    public:
        static std::vector<VulkanRenderGraphRenderpassResources*> create(
            RenderResourcesSystem& renderResourcesSystem,
            const std::vector<hl::RenderpassInfo>& renderpassInfo, 
            VulkanDevice& device,
            uint32_t width,
            uint32_t height,
            const std::vector<VkImageView>& swapChainImageViews);

        static void destroy(std::vector<VulkanRenderGraphRenderpassResources*>& generatedRenderpassResources);

        static VkFormat extractFormat(const std::string& formatString);
        static VkDescriptorType extractDescriptorType(const std::string& descriptorTypeString);
        static VkFormat extractVertexAttributeFormat(VertexAttributeFormat format);
    };

}