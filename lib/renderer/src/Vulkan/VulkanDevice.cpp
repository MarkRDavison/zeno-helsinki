#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <stdexcept>
#include <iostream>
#include <set>

namespace hl
{
    VulkanDevice::VulkanDevice(
        VulkanInstance& instance,
        VulkanSurface& surface
    ) :
        _instance(instance),
        _surface(surface)
    {

    }

	void VulkanDevice::create()
	{
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void VulkanDevice::destroy()
	{
        vkDestroyDevice(_device, nullptr);
	}
    void VulkanDevice::waitIdle()
    {
        vkDeviceWaitIdle(_device);
    }

    uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

	void VulkanDevice::pickPhysicalDevice()
	{
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance._instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance._instance, &deviceCount, devices.data());

        for (const auto& d : devices)
        {
            if (isDeviceSuitable(d, _surface._surface))
            {
                _physicalDevice = d;
                _msaaSamples = getMaxUsableSampleCount(_physicalDevice);
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
	}

	void VulkanDevice::createLogicalDevice()
	{
        auto queueIndices = VulkanQueue::findQueueFamilies(_physicalDevice, _surface._surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(_device, queueIndices.graphicsFamily.value(), 0, &_graphicsQueue._queue);
        vkGetDeviceQueue(_device, queueIndices.presentFamily.value(), 0, &_presentQueue._queue);
	}

    bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice d, VkSurfaceKHR s)
    {
        auto queueIndices = VulkanQueue::findQueueFamilies(d, s);

        bool extensionsSupported = checkDeviceExtensionSupport(d);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            auto swapChainSupport = VulkanSwapChain::querySwapChainSupport(d, s);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(d, &supportedFeatures);

        return queueIndices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice p)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(p, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(p, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount(VkPhysicalDevice p)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(p, &physicalDeviceProperties);

        uint32_t maxPushConstantSize = physicalDeviceProperties.limits.maxPushConstantsSize;

        // TODO: Record this max size somewhere
        std::cout << "Max push constant size: " << maxPushConstantSize << std::endl;

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }
}