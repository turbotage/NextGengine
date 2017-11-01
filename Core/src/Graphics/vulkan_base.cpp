#include "vulkan_base.h"
#include "../debug.h"
#include <iostream>
#include <map>
#include <utility>
#include <set>

const std::vector<const char*> graphicsDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> computeDeviceExtensions = {

};

ng::graphics::QueueFamilyIndices ng::graphics::VulkanBase::findQueueFamilies(VkPhysicalDevice device) {
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
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}
		if (indices.isComputeComplete() && indices.isGraphicsComplete()) {
			break;
		}
		i++;
	}
	return indices;
}

ng::graphics::SwapChainSupportDetails ng::graphics::VulkanBase::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
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
	auto checkDeviceExtensionSupport = [&](VkPhysicalDevice device, short deviceType) -> bool {
		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions;
		if (deviceType == GRAPHICS_UNIT) {
			std::set<std::string> requiredExtensions2(graphicsDeviceExtensions.begin(), graphicsDeviceExtensions.end());
			requiredExtensions = requiredExtensions2;
		}
		else if (deviceType == COMPUTE_UNIT) {
			std::set<std::string> requiredExtensions2(computeDeviceExtensions.begin(), computeDeviceExtensions.end());
			requiredExtensions = requiredExtensions2;
		}
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();

	};

	auto rateGraphicsDeviceSuitability = [&](VkPhysicalDevice device, bool needComputeCompatibility) -> uint32 {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;

		//things that is needed will return 0 if they don't exist
		//return 0 if no gemomatryShader is available
		if (!deviceFeatures.geometryShader) {
			return 0;
		}
		QueueFamilyIndices indices = findQueueFamilies(device);
		//if it doesn't support graphicsQueue return 0
		if (!indices.isGraphicsComplete()) {
			return 0;
		}
		//if it doesn't support presentQueue return 0
		if (!indices.isPresentComplete()) {
			return 0;
		}
		//if the graphics device also have to be the compute device return 0 if computeQueue isn't available
		if (needComputeCompatibility && !indices.isComputeComplete()) {
			return 0;
		}
		//check if graphicsDevice extensions are available, if not return 0
		if (!checkDeviceExtensionSupport(device, GRAPHICS_UNIT)) {
			return 0;
		}
		//if the graphicsDevice also needs to be computeDevice check if it also have computeDevice extensions
		//otherwise return 0
		if (needComputeCompatibility) {
			if (!checkDeviceExtensionSupport(device, COMPUTE_UNIT)) {
				return 0;
			}
		}
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		if (swapChainSupport.formats.empty() && swapChainSupport.presentModes.empty()) {
			return 0;
		}

		//score adding stuff
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		score += (int)(deviceProperties.limits.maxImageDimension2D / 2);
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		printf("rate graphics device\n");
		printf("MemHeapCount : %d\n", memProperties.memoryHeapCount);
		printf("MemTypeCount : %d\n", memProperties.memoryTypeCount);
		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			printf("HeapIndex : %d\n", i);
			printf("HeapSize : %" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			printf("HeapFlags : %d\n", memProperties.memoryHeaps[i].flags);
			
			for (int j = 0; j < memProperties.memoryTypeCount; ++j) {
				if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					//Memory Type 0
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT\n");
						score += memProperties.memoryHeaps[i].size / 10000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
					//Memory Type 1
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
						score += memProperties.memoryHeaps[i].size / 30000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
					//MemoryType 2
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
						score += memProperties.memoryHeaps[i].size / 50000;
					}
				}
				else if (memProperties.memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
					if (memProperties.memoryTypes[j].heapIndex == i) {
						printf("Found VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT\n");
						score += memProperties.memoryHeaps[i].size / 80000;
					}
				}

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
		printf("rate compute device\n");
		for (int i = 0; i < memProperties.memoryHeapCount; ++i) {
			/*
			printf("%" PRIu64 "\n", memProperties.memoryHeaps[i].size);
			printf("%d\n\n", memProperties.memoryHeaps[i].flags);
			printf("%d\n\n", memProperties.memoryTypes[i].propertyFlags);
			printf("%d\n\n", memProperties.memoryTypes[i].heapIndex);
			if (memProperties.memoryHeaps[i].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 1000000);
				}
				else if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 3000000);
				}
				else if (memProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
					score += (int)(memProperties.memoryHeaps[i].size / 5000000);
				}
				else {
					return 0;
				}
			}
			else {
				printf("found non VK_MEMORY_HEAP_DEVICE_LOCAL_BIT\n");
			}
			*/
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

	vkGetPhysicalDeviceMemoryProperties(graphicsUnit.pDevice.device , &graphicsUnit.pDevice.memoryProperties);
	vkGetPhysicalDeviceMemoryProperties(computeUnit.pDevice.device, &computeUnit.pDevice.memoryProperties);
}

void ng::graphics::VulkanBase::createLogicalDevices()
{
	//graphicsUnit
	{
		QueueFamilyIndices indices = findQueueFamilies(graphicsUnit.pDevice.device);
		
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32>(graphicsDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = graphicsDeviceExtensions.data();

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
		vkGetDeviceQueue(graphicsUnit.device, indices.presentFamily, 0, &graphicsUnit.presentQueue);
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
		createInfo.enabledExtensionCount = static_cast<uint32>(computeDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = computeDeviceExtensions.data();

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

