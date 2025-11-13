#include <helsinki/Renderer/Vulkan/VulkanQueue.hpp>
#include <iostream>
#include <vector>

namespace hl
{
    VulkanQueueFamilyIndices VulkanQueue::findQueueFamilies(VkPhysicalDevice d, VkSurfaceKHR s)
    {
        VulkanQueueFamilyIndices indices{};

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &count, nullptr);

        std::vector<VkQueueFamilyProperties> families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &count, families.data());

        std::optional<uint32_t> fallbackTransferFamily;

        for (uint32_t i = 0; i < count; ++i)
        {
            const auto& q = families[i];

            if ((q.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !indices.graphicsFamily)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, s, &presentSupport);
            if (presentSupport && !indices.presentFamily)
            {
                indices.presentFamily = i;
            }


            if ((q.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(q.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.computeFamily = i; // dedicated
            }
            else if ((q.queueFlags & VK_QUEUE_COMPUTE_BIT) && !indices.computeFamily)
            {
                indices.computeFamily = i; // fallback
            }

            if ((q.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(q.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(q.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.transferFamilies.push_back(i);
            }
            else if ((q.queueFlags & VK_QUEUE_TRANSFER_BIT) && !fallbackTransferFamily.has_value())
            {
                fallbackTransferFamily = i;
            }
        }

        if (indices.transferFamilies.empty() && fallbackTransferFamily.has_value())
        {
            indices.transferFamilies.push_back(fallbackTransferFamily.value());
        }

        return indices;
    }

}