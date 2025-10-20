#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace hl
{
    struct VulkanSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
    };
}