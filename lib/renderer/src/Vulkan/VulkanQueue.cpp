#include <helsinki/Renderer/Vulkan/VulkanQueue.hpp>
#include <vector>

namespace hl
{
    VulkanQueueFamilyIndices VulkanQueue::findQueueFamilies(VkPhysicalDevice d, VkSurfaceKHR s)
    {
        VulkanQueueFamilyIndices queueIndices{};

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueIndices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, s, &presentSupport);

            if (presentSupport)
            {
                queueIndices.presentFamily = i;
            }

            if (queueIndices.isComplete())
            {
                break;
            }

            i++;
        }

        return queueIndices;
    }
}