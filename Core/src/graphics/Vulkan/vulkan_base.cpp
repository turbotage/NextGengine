#include "vulkan_base.h"
#include "../../debug.h"
#include <iostream>
#include <map>
#include <utility>

void ng::graphics::VulkanBase::createInstance()
{
	bool validationLayersEnabled = ng::graphics::debug::isValidationLayersEnabled();
	if (validationLayersEnabled && !ng::graphics::debug::checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	ng::graphics::debug::setDebugExtensions(this);
	createInfo. enabledExtensionCount= static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (validationLayersEnabled) {
		ng::graphics::debug::setDebugValidationLayers(this);
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void ng::graphics::VulkanBase::freeInstance()
{
	vkDestroyInstance(instance, nullptr);
}

void ng::graphics::VulkanBase::createDebugCallback()
{
	debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debug::setupDebugging(instance, debugReportFlags, debugReportCallback);
}

void ng::graphics::VulkanBase::freeDebugCallback()
{
	debug::freeDebugCallback(instance, debugReportCallback);
}

void ng::graphics::VulkanBase::createDevices()
{
	auto rateGraphicsDeviceSuitability = [&](VkPhysicalDevice device) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += (int)(deviceProperties.limits.maxImageDimension2D / 2);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			//printf("%" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			//printf("%d\n\n", memProperties.memoryHeaps[i].flags);
			if (memProperties.memoryHeaps[i].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				score += (int)(memProperties.memoryHeaps[i].size / 100000);
			}
			else {
				score += (int)(memProperties.memoryHeaps[i].size / 300000);
			}
		}

		printf("physicalDevice score: %d\n", score);
		return score;
	};

	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	auto pickGraphicsDevice = [&](std::vector<VkPhysicalDevice>* devicesIn) -> VkPhysicalDevice {
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : *devicesIn) {
			int score = rateGraphicsDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0) {
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable GPU");
		}
	};

	graphicsUnit.pDevice.device = pickGraphicsDevice(&devices);

}
