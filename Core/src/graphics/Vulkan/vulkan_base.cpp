#include "vulkan_base.h"
#include "../../debug.h"
#include <iostream>
#include <map>
#include <utility>

ng::graphics::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	ng::graphics::QueueFamilyIndices indices;

	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
		}
		if (indices.isComputeComplete() && indices.isGraphicsComplete()) {
			break;
		}
		i++;
	}
	return indices;
}

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

void ng::graphics::VulkanBase::createPhysicalDevices()
{
	auto rateGraphicsDeviceSuitability = [&](VkPhysicalDevice device, bool needComputeCompatibility) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;
		if (!deviceFeatures.geometryShader) {
			return 0;
		}
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isGraphicsComplete()) {
			return 0;
		}
		if (needComputeCompatibility && !indices.isComputeComplete()) {
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
				score += (int)(memProperties.memoryHeaps[i].size / 1000000);
			}
			else {
				score += (int)(memProperties.memoryHeaps[i].size / 3000000);
			}
		}
		printf("physicalDevice score: %d\n", score);
		return score;
	};

	auto rateComputeDeviceSuitability = [&](VkPhysicalDevice device) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isComputeComplete()) {
			return 0;
		}
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			//printf("%" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			//printf("%d\n\n", memProperties.memoryHeaps[i].flags);
			if (memProperties.memoryHeaps[i].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				score += (int)(memProperties.memoryHeaps[i].size / 1000000);
			}
			else {
				score += (int)(memProperties.memoryHeaps[i].size / 3000000);
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

	printf("Number of PhysicalDevices: %" PRIu32 "\n", deviceCount);

	auto pickGraphicsDevice = [&](std::vector<VkPhysicalDevice>* devicesIn, bool needComputeCapability = false) -> VkPhysicalDevice {
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : *devicesIn) {
			int score = rateGraphicsDeviceSuitability(device, needComputeCapability);
			candidates.insert(std::make_pair(score, device));
		}
		if (candidates.rbegin()->first > 0) {
			if (needComputeCapability) {
				computeAndGraphicsSameDevice = true;
			}
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable graphicsUnit");
		}
	};

	auto pickComputeDevice = [&](std::vector<VkPhysicalDevice>* devicesIn) -> VkPhysicalDevice {
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : *devicesIn) {
			int score = rateComputeDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
		if (candidates.rbegin()->first > 0) {
			return candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable computeUnit");
		}
	};

	//printf("DeviceCount: %d\n", deviceCount);
	if (!(deviceCount > 1)) {
		graphicsUnit.pDevice.device = pickGraphicsDevice(&devices, true);
		computeUnit.pDevice.device = graphicsUnit.pDevice.device;
	}
	else {
		graphicsUnit.pDevice.device = pickGraphicsDevice(&devices);
		devices.erase(std::find(devices.begin(), devices.end(), graphicsUnit.pDevice.device));
		computeUnit.pDevice.device = pickComputeDevice(&devices);
	}

}

void ng::graphics::VulkanBase::createLogicalDevices()
{
	//graphicsUnit
	{
		QueueFamilyIndices indices = findQueueFamilies(graphicsUnit.pDevice.device);

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		if (ng::graphics::debug::isValidationLayersEnabled()) {
			createInfo.enabledLayerCount = static_cast<uint32>(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(graphicsUnit.pDevice.device, &createInfo, nullptr, &graphicsUnit.device)) {
			throw std::runtime_error("failed to create logical graphicsUnit");
		}

		vkGetDeviceQueue(graphicsUnit.device, indices.graphicsFamily, 0, &graphicsUnit.graphicsQueue);
	}
	//computeUnit
	{
		QueueFamilyIndices indices = findQueueFamilies(computeUnit.pDevice.device);

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.computeFamily;
		queueCreateInfo.queueCount = 1;

		float queuePriority = 0.6f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		if (ng::graphics::debug::isValidationLayersEnabled()) {
			createInfo.enabledLayerCount = static_cast<uint32>(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(computeUnit.pDevice.device, &createInfo, nullptr, &computeUnit.device)) {
			throw std::runtime_error("failed to create logical graphicsUnit");
		}

		vkGetDeviceQueue(computeUnit.device, indices.computeFamily, 0, &computeUnit.computeQueue);
	}
}

void ng::graphics::VulkanBase::freeLogicalDevices()
{
	vkDestroyDevice(graphicsUnit.device, nullptr);
	vkDestroyDevice(computeUnit.device, nullptr);
}

