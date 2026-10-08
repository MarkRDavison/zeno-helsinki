#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>
#include <helsinki/Renderer/Vulkan/VulkanSwapChain.hpp>
#include <helsinki/Renderer/RendererConfiguration.hpp>
#include <cassert>
#include <stdexcept>
#include <set>
#include <string>

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

	static void queryModernDeviceFeatures(
		VkPhysicalDevice physicalDevice,
		VkPhysicalDeviceFeatures2& features2,
		VkPhysicalDeviceVulkan12Features& vulkan12,
		VkPhysicalDeviceVulkan13Features& vulkan13)
	{
		vulkan13 = {};
		vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

		vulkan12 = {};
		vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12.pNext = &vulkan13;

		features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &vulkan12;

		vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
	}

	static std::string missingRequiredModernFeatures(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		if (VK_VERSION_MAJOR(properties.apiVersion) < 1
			|| (VK_VERSION_MAJOR(properties.apiVersion) == 1
				&& VK_VERSION_MINOR(properties.apiVersion) < 3))
		{
			return "Vulkan 1.3 apiVersion";
		}

		VkPhysicalDeviceFeatures2 features2{};
		VkPhysicalDeviceVulkan12Features vulkan12{};
		VkPhysicalDeviceVulkan13Features vulkan13{};
		queryModernDeviceFeatures(physicalDevice, features2, vulkan12, vulkan13);

		if (vulkan13.dynamicRendering != VK_TRUE)
		{
			return "dynamicRendering";
		}
		if (vulkan13.synchronization2 != VK_TRUE)
		{
			return "synchronization2";
		}
		if (vulkan12.timelineSemaphore != VK_TRUE)
		{
			return "timelineSemaphore";
		}
		if (vulkan12.descriptorIndexing != VK_TRUE)
		{
			return "descriptorIndexing";
		}
		if (vulkan12.shaderSampledImageArrayNonUniformIndexing != VK_TRUE)
		{
			return "shaderSampledImageArrayNonUniformIndexing";
		}
		if (vulkan12.descriptorBindingPartiallyBound != VK_TRUE)
		{
			return "descriptorBindingPartiallyBound";
		}
		if (vulkan12.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE)
		{
			return "descriptorBindingSampledImageUpdateAfterBind";
		}
		if (vulkan12.runtimeDescriptorArray != VK_TRUE)
		{
			return "runtimeDescriptorArray";
		}

		return {};
	}

	bool VulkanDevice::isBasicallySuitable(VkPhysicalDevice d, VkSurfaceKHR s)
	{
		auto queueIndices = VulkanQueue::findQueueFamilies(d, s);

		bool extensionsSupported = checkDeviceExtensionSupport(d);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			auto swapChainSupport = VulkanSwapChain::querySwapChainSupport(d, s);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures{};
		vkGetPhysicalDeviceFeatures(d, &supportedFeatures);

		return queueIndices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	void VulkanDevice::pickPhysicalDevice()
	{
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance.handle(), &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance.handle(), &deviceCount, devices.data());

		std::string lastMissingFeature;

        for (const auto& d : devices)
        {
            if (!isBasicallySuitable(d, _surface.handle()))
            {
                continue;
            }

			const auto missing = missingRequiredModernFeatures(d);
			if (!missing.empty())
			{
				lastMissingFeature = missing;
				continue;
			}

			_physicalDevice = d;
			_msaaSamples = getMaxUsableSampleCount(_physicalDevice);
			break;
        }

        if (_physicalDevice == VK_NULL_HANDLE)
        {
			if (!lastMissingFeature.empty())
			{
				throw std::runtime_error(
					"failed to find a suitable GPU: missing required feature " + lastMissingFeature);
			}

            throw std::runtime_error("failed to find a suitable GPU!");
        }
	}

	void VulkanDevice::createLogicalDevice()
	{
        auto queueIndices = VulkanQueue::findQueueFamilies(_physicalDevice, _surface.handle());

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

		VkPhysicalDeviceVulkan13Features vulkan13{};
		vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13.dynamicRendering = VK_TRUE;
		vulkan13.synchronization2 = VK_TRUE;

		VkPhysicalDeviceVulkan12Features vulkan12{};
		vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12.pNext = &vulkan13;
		vulkan12.timelineSemaphore = VK_TRUE;
		vulkan12.descriptorIndexing = VK_TRUE;
		vulkan12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		vulkan12.descriptorBindingPartiallyBound = VK_TRUE;
		vulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		vulkan12.runtimeDescriptorArray = VK_TRUE;

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &vulkan12;
		features2.features.samplerAnisotropy = VK_TRUE;
		features2.features.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &features2;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = nullptr;

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

        CHECK_VK_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));

        vkGetDeviceQueue(_device, queueIndices.graphicsFamily.value(), 0, &_graphicsQueue._queue);
        vkGetDeviceQueue(_device, queueIndices.presentFamily.value(), 0, &_presentQueue._queue);

		loadCore13Commands();
	}

	void VulkanDevice::loadCore13Commands()
	{
		assert(_device != VK_NULL_HANDLE);

		_cmdBeginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(
			vkGetDeviceProcAddr(_device, "vkCmdBeginRendering"));
		if (_cmdBeginRendering == nullptr)
		{
			_cmdBeginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(
				vkGetDeviceProcAddr(_device, "vkCmdBeginRenderingKHR"));
		}

		_cmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(
			vkGetDeviceProcAddr(_device, "vkCmdEndRendering"));
		if (_cmdEndRendering == nullptr)
		{
			_cmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(
				vkGetDeviceProcAddr(_device, "vkCmdEndRenderingKHR"));
		}

		_cmdPipelineBarrier2 = reinterpret_cast<PFN_vkCmdPipelineBarrier2>(
			vkGetDeviceProcAddr(_device, "vkCmdPipelineBarrier2"));
		if (_cmdPipelineBarrier2 == nullptr)
		{
			_cmdPipelineBarrier2 = reinterpret_cast<PFN_vkCmdPipelineBarrier2>(
				vkGetDeviceProcAddr(_device, "vkCmdPipelineBarrier2KHR"));
		}

		assert(_cmdBeginRendering != nullptr);
		assert(_cmdEndRendering != nullptr);
		assert(_cmdPipelineBarrier2 != nullptr);
	}

    bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice d, VkSurfaceKHR s)
    {
		if (!isBasicallySuitable(d, s))
		{
			return false;
		}

		const auto missing = missingRequiredModernFeatures(d);
		if (!missing.empty())
		{
			throw std::runtime_error(
				"GPU is not suitable: missing required feature " + missing);
		}

		return true;
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

        //uint32_t maxPushConstantSize = physicalDeviceProperties.limits.maxPushConstantsSize;
        // TODO: Record this max size somewhere
        
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