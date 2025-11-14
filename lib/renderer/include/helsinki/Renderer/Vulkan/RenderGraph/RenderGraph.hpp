#pragma once

#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <helsinki/Renderer/Vulkan/VulkanImage.hpp>
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
        std::optional<VkClearValue> clear;
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

    struct PipelineInfo
    {
        std::string name;
        std::string shaderVert;
        std::string shaderFrag;
        std::vector<DescriptorSetInfo> descriptorSets;
        std::optional<VertexInputInfo> vertexInputInfo;
        DepthState depthState;
        RasterState rasterState;
        bool enableBlending = false;
    };

    struct RenderpassInfo
    {
        std::string name;
        bool useMultiSampling;// TODO: Move this to the outputs ResourceInfo type
        std::vector<std::string> inputs;           // names of input resources
        std::vector<ResourceInfo> outputs;         // resources this pass writes
        std::vector<PipelineInfo> pipelines;       // pipelines in this pass
        VkExtent2D extent{};
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
            const std::vector<hl::RenderpassInfo>& renderpassInfo, 
            VulkanDevice& device,
            uint32_t width,
            uint32_t height,
            const std::vector<VkImageView>& swapChainImageViews);

        static void destroy(std::vector<VulkanRenderGraphRenderpassResources*>& generatedRenderpassResources);

        static void createImages(
            VulkanDevice& device,
            VulkanRenderGraphRenderpassResources* resources,
            const RenderpassInfo& info,
            uint32_t width,
            uint32_t height,
            uint32_t imageCount,
            bool isLastRenderpass);
        static void createFrameBuffers(
            VulkanDevice& device,
            VulkanRenderGraphRenderpassResources *resources, 
            const RenderpassInfo& info,
            uint32_t width,
            uint32_t height,
            const std::vector<VkImageView>& swapChainImageViews,
            uint32_t imageCount,
            bool isLastRenderpass);

        static VkFormat extractFormat(const std::string& formatString);
        static VkDescriptorType extractDescriptorType(const std::string& descriptorTypeString);
        static VkFormat extractVertexAttributeFormat(VertexAttributeFormat format);
    };

}